import zlib

# Create one 4-byte word with the value 0xAABBCCDD.
# Here we choose big-endian byte order (you could also use little-endian if desired).
word = (0xAABBCCDD).to_bytes(4, byteorder='big')

# Build the buffer by repeating this word 64 times (64*4 = 256 bytes)
buffer = word * 64

# Compute the CRC32.
# zlib.crc32 computes the standard CRC-32 (IEEE 802.3) checksum.
# The standard CRC-32 algorithm uses:
#   - Polynomial: 0x04C11DB7 (bit-reversed as 0xEDB88320 in many implementations)
#   - Initial seed: 0xFFFFFFFF
#   - Final XOR value: 0xFFFFFFFF
crc_value = zlib.crc32(buffer) & 0xFFFFFFFF

print("CRC32: 0x{:08X}".format(crc_value))
