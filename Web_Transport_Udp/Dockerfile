FROM rust:1.95-bullseye AS builder

WORKDIR /app
RUN apt-get update && apt-get install -y pkg-config libssl-dev && rm -rf /var/lib/apt/lists/*
COPY . .
RUN cargo build --release

FROM debian:bookworm-slim

WORKDIR /app
RUN apt-get update && apt-get install -y ca-certificates && rm -rf /var/lib/apt/lists/*
COPY --from=builder /app/target/release/webtransport_proxy /app/webtransport_proxy

EXPOSE 443/tcp
EXPOSE 443/udp

CMD ["./webtransport_proxy"]
