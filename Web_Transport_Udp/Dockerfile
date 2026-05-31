# 1. Planer stage: Creates the recipe for the dependencies
FROM rust:1.95-bullseye AS planner
WORKDIR /app
RUN cargo install cargo-chef
COPY . .
RUN cargo chef prepare --recipe-path recipe.json

# 2. Cacher stage: Builds on the dependencies based on the recipe
FROM rust:1.95-bullseye AS cacher
WORKDIR /app
RUN cargo install cargo-chef
RUN apt-get update && apt-get install -y pkg-config libssl-dev && rm -rf /var/lib/apt/lists/*
COPY --from=planner /app/recipe.json recipe.json
RUN cargo chef cook --release --recipe-path recipe.json

# 3. Builder stage: Copies the real source code and uses the cache from stage 2
FROM rust:1.95-bullseye AS builder
WORKDIR /app
RUN apt-get update && apt-get install -y pkg-config libssl-dev && rm -rf /var/lib/apt/lists/*
COPY . .
# Copies the previously compiled dependencies
COPY --from=cacher /app/target target
COPY --from=cacher /usr/local/cargo /usr/local/cargo
RUN cargo build --release

# 4. Final, minimal runtime stage
FROM debian:bookworm-slim
WORKDIR /app
RUN apt-get update && apt-get install -y ca-certificates && rm -rf /var/lib/apt/lists/*
COPY --from=builder /app/target/release/webtransport_proxy /app/webtransport_proxy

EXPOSE 443/tcp
EXPOSE 443/udp

CMD ["./webtransport_proxy"]
