FROM ubuntu:22.04

# Avoid interactive prompts during apt package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install g++, make, valgrind, and gdb required for compilation and practical demonstration
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    valgrind \
    gdb \
    doxygen \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy all project files into container
COPY . .

# Build the project using the Makefile (both tividy and tividy_tests)
RUN make clean && make all

# Default command runs the compiled application
CMD ["./tividy"]
