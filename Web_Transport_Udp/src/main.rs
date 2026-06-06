use std::env;
use std::net::SocketAddr;
use std::sync::Arc;
use anyhow::{Context, Result};
use pretty_hex::PrettyHex;
use tokio::net::UdpSocket;
// Change Certificate to Identity here:
use wtransport::{Endpoint, ServerConfig, Identity};

#[tokio::main]
async fn main() -> Result<()> {
    // 1. Parse configuration from environment variables (defined in docker-compose)
    let listen_addr: SocketAddr = "0.0.0.0:443".parse()?;
    let target_host = env::var("TARGET_UDP_HOST").unwrap_or_else(|_| "udp-backend".to_string());
    let target_port = env::var("TARGET_UDP_PORT").unwrap_or_else(|_| "5005".to_string());
    let target_addr_str = format!("{}:{}", target_host, target_port);
    let verbosity: i32 = env::var("WT_VERBOSE")
        .expect("WT_VERBOSE environment variable is not set")
        .parse()
        .expect("WT_VERBOSE is not a valid integer");

    println!("Starting WebTransport proxy on {}", listen_addr);
    println!("Target UDP Backend: {}", target_addr_str);

    // 2. Load TLS Certificates (Required for WebTransport/QUIC)
    // Identity::load_pemfiles is async and reads your files directly from disk
    let identity = Identity::load_pemfiles(
        "/etc/ssl/certs/cert.pem", 
        "/etc/ssl/certs/key.pem"
    )
    .await  // <-- Crucial: This method requires an .await block
    .context("Failed to load TLS identity files from /etc/ssl/certs/")?;

    let config = ServerConfig::builder()
        .with_bind_address(listen_addr)
        .with_identity(identity) // <-- Pass the loaded identity struct here
        .build();

    // 3. Bind the UDP/QUIC Endpoint
    let server = Endpoint::server(config)?;

    // 4. Accept incoming connection loops
    loop {
        let incoming_session = server.accept().await;
        let target_backend = target_addr_str.clone();

        tokio::spawn(async move {
            if let Err(e) = handle_session(incoming_session, target_backend, verbosity).await {
                eprintln!("Session error: {:?}", e);
            }
        });
    }
}

async fn handle_session(
        incoming_session: wtransport::endpoint::IncomingSession, 
        target_backend: String,
        verbosity: i32) -> Result<()> {
    // Validate and accept the WebTransport handshake
    let session_request = incoming_session.await?;
    let connection = session_request.accept().await?;
    println!("New WebTransport connection accepted from: {}", connection.remote_address());

    // Bind a unique local UDP socket for this specific client session to handle responses
    let udp_socket = Arc::new(UdpSocket::bind("0.0.0.0:0").await.context("Failed to bind local UDP socket")?);
    
    // Connect the socket to the backend service to isolate incoming traffic to this sender
    udp_socket.connect(&target_backend).await.context("Failed to connect to backend UDP service")?;
    
    let udp_socket_recv = Arc::clone(&udp_socket);
    let connection = Arc::new(connection);
    let connection_clone = Arc::clone(&connection);

    // --- TASK 1: WebTransport Datagrams -> UDP Backend ---
    let wt_to_udp = tokio::spawn(async move {
        loop {
            // Receive a datagram payload from the WebTransport client
            match connection.receive_datagram().await {
                Ok(payload) => {
                    if let Err(e) = udp_socket.send(&payload).await {
                        eprintln!("WT -> UDP: Error: {:?}", e);
                        eprintln!("Buffer content ({} bytes):\n{:?}", payload.len(), payload.hex_dump());
                        break;
                    }
                    if verbosity >= 1 {
                        eprintln!("WT -> UDP: Success");
                        if verbosity >= 2 {
                            eprintln!("Buffer content ({} bytes):\n{:?}", payload.len(), payload.hex_dump());
                        }
                    }
                }
                Err(_) => {
                    println!("WebTransport client dropped or connection closed.");
                    break;
                }
            }
        }
    });

    // --- TASK 2: UDP Backend -> WebTransport Datagrams ---
    let udp_to_wt = tokio::spawn(async move {
        let mut buffer = [0u8; 65535]; // Max UDP packet size
        loop {
            match udp_socket_recv.recv(&mut buffer).await {
                Ok(bytes_read) => {
                    if bytes_read == 0 { break; }
                    let payload = &buffer[..bytes_read];
                    // Push the response chunk directly back to the WebTransport stream
                    if let Err(e) = connection_clone.send_datagram(&payload) {
                        eprintln!("UDP -> WT: Error: {:?}", e);
                        eprintln!("Buffer content ({} bytes):\n{:?}", payload.len(), payload.hex_dump());
                        break;
                    }
                    if verbosity >= 1 {
                        eprintln!("UDP -> WT: Success");
                        if verbosity >= 2 {
                            eprintln!("Buffer content ({} bytes):\n{:?}", payload.len(), payload.hex_dump());
                        }
                    }
                }
                Err(e) => {
                    eprintln!("Error reading from UDP backend: {:?}", e);
                    break;
                }
            }
        }
    });

    // Keep session tasks alive until one side disconnects or crashes
    tokio::select! {
        _ = wt_to_udp => {},
        _ = udp_to_wt => {},
    };

    println!("Session cleanly terminated.");
    Ok(())
}
