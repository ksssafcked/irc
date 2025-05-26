FROM debian:bullseye

# Install exact Irssi version (1.2.3-1)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    irssi \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m ircuser
USER ircuser
WORKDIR /home/ircuser

ENTRYPOINT ["irssi"]
