# Directories
RINDEX_DIR = r-index
BUILD_DIR = $(RINDEX_DIR)/build
MAIN_DIR = .

# Executables to copy from r-index/build
RINDEX_EXECUTABLES = ri-build ri-count ri-locate ri-space

# Default target
all: ri-lcp r-index-build copy-executables

# Build ri-lcp executable
ri-lcp: ri-lcp.cpp
	g++ ri-lcp.cpp -llcptools -o ri-lcp

# Build r-index tools
r-index-build: $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Copy executables from r-index/build to main directory
copy-executables: r-index-build
	@for exe in $(RINDEX_EXECUTABLES); do \
		if [ -f $(BUILD_DIR)/$$exe ]; then \
			cp $(BUILD_DIR)/$$exe $(MAIN_DIR)/; \
		fi \
	done

# Clean build artifacts
clean:
	rm -f ri-lcp
	rm -f $(RINDEX_EXECUTABLES)
	rm -rf $(BUILD_DIR)

# Clean only r-index build
clean-r-index:
	rm -rf $(BUILD_DIR)
	rm -f $(RINDEX_EXECUTABLES)

# Clean only ri-lcp
clean-ri-lcp:
	rm -f ri-lcp

# Rebuild everything from scratch
rebuild: clean all

.PHONY: all ri-lcp r-index-build copy-executables clean clean-r-index clean-ri-lcp rebuild

