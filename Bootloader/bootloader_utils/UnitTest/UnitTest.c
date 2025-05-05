#include "bootloader_utils/bl_decompress/bl_decompress.h"
#include "bootloader_utils/bl_signature/bl_hash/sha256.h"
#include "bootloader_utils/bl_signature/bl_signature.h"      

#define TEST_DECOMPRESS 0
#define TEST_SHA256 0
#define TEST_INCREMENTAL 1
#define TEST_ECDSA 0
#define TEST_VERIFICATION_WRAPPER 0

#define DECOMPRESSED_SIZE 1020
#define COMPRESSED_SIZE 65
const uint8_t compressed_data[] = {
		0xff, 0x24, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x66, 0x72, 0x6f, 0x6d, 0x20, 0x50, 0x79, 0x74, 0x68, 0x6f, 0x6e, 0x21, 0x20, 0x54, 0x68, 0x69, 0x73, 0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x62, 0x65, 0x20, 0x64, 0x65, 0x63, 0x6f, 0x6d, 0x70, 0x72, 0x65, 0x73, 0x73, 0x65, 0x64, 0x20, 0x69, 0x6e, 0x20, 0x43, 0x2e, 0x20, 0x33, 0x00, 0xff, 0xff, 0xff, 0xb4, 0x50, 0x6e, 0x20, 0x43, 0x2e, 0x20
};
char decompressed_buf[DECOMPRESSED_SIZE];
char decompressed_buf2[DECOMPRESSED_SIZE];
int main() {

#if TEST_DECOMPRESS
        uint32_t result = BLDecomp_Decompress(
			(const uint8_t*)compressed_data,
			(uint8_t*)decompressed_buf,
			COMPRESSED_SIZE);

	if (result<0) {
        while(1);
	}

	//check decompressed buffer to match message result = 1020 bytes, Message : "Hello from Python! This will be decompressed in C."*20"
#endif

#if TEST_SHA256
	// sha256_c_test.c

#if !TEST_INCREMENTAL
	// 1. Define the same dummy message
	const unsigned char message[] = "Dummy test data for SHA256\n";
	size_t message_len = strlen((const char *)message);

	// 2. Prepare context and output buffer
	SHA256_CTX ctx;                // context struct :contentReference[oaicite:5]{index=5}
	unsigned char digest[32];      // SHA-256 outputs 32 bytes :contentReference[oaicite:6]{index=6}

	// 3. Compute SHA-256 digest incrementally
	sha256_init(&ctx);                             // init :contentReference[oaicite:7]{index=7}
	sha256_update(&ctx, message, message_len);     // update with data chunk :contentReference[oaicite:8]{index=8}
	sha256_final(&ctx, digest);                    // finalize into digest :contentReference[oaicite:9]{index=9}

	// 4. check hex-encoded digest

#else
	// Three distinct data chunks
	uint8_t chunk1[] = { 0x10, 0x20, 0x30 };
	uint8_t chunk2[] = { 0xAA, 0xBB, 0xCC, 0xDD };
	uint8_t chunk3[] = "Hello !\n";

	SHA256_CTX ctx;                  // context struct :contentReference[oaicite:8]{index=8}
	uint8_t digest[32];              // 32‑byte digest output :contentReference[oaicite:9]{index=9}

	// Incremental hashing sequence
	sha256_init(&ctx);                                   // initialize :contentReference[oaicite:10]{index=10}
	sha256_update(&ctx, chunk1, sizeof(chunk1));         // process chunk1 :contentReference[oaicite:11]{index=11}
	sha256_update(&ctx, chunk2, sizeof(chunk2));         // process chunk2 :contentReference[oaicite:12]{index=12}
	sha256_update(&ctx, chunk3, strlen((char*)chunk3));   // process chunk3 :contentReference[oaicite:13]{index=13}
	sha256_final(&ctx, digest);                          // finalize and output digest :contentReference[oaicite:14]{index=14}

	// check hex-encoded digest

#endif/*Test Incremental #if*/
#endif/*Test SHA256 #if*/

#if TEST_ECDSA

	// 1. Exact same message as in Python
	uint8_t message[] = "Hello, MPC5748G! This is a test for ECDSA signature verification made by the SDV SOTA Team";

	// 3. Signature (64 bytes):
	uint8_t signature[64] = { 0x79, 0x1c, 0x4e, 0xe6, 0xcb, 0x89, 0x23, 0x1e, 0x88, 0xf5, 0x09, 0xa0, 0xb7, 0x6a, 0x37, 0x6f, 0xae, 0x48, 0x0e, 0x89, 0xe7, 0x2e, 0x71, 0xe5, 0xd5, 0x0d, 0x1b, 0xde, 0xb1, 0xea, 0x7f, 0x94, 0x09, 0xde, 0x32, 0xff, 0xfd, 0x0d, 0xbd, 0x2b, 0xf5, 0x99, 0xc9, 0xbe, 0x02, 0x8c, 0x6a, 0xce, 0x82, 0xe6, 0x19, 0xf8, 0x5c, 0xe0, 0xac, 0xb8, 0xce, 0x07, 0xb7, 0x92, 0x92, 0x89, 0x6d, 0x3f };

	uint8_t digest[32];
#if TEST_VERIFICATION_WRAPPER
	status_t returnCode =STATUS_SUCCESS;
	returnCode = BLSig_InitHash();
	if(returnCode==STATUS_ERROR){
		while(1);
	}
	returnCode = BLSig_UpdateHash(message,sizeof(message));
	if(returnCode==STATUS_ERROR){
		while(1);
	}
	returnCode = BLSig_FinalizeHash(digest);
	if(returnCode==STATUS_ERROR){
		while(1);
	}
	returnCode = BLSig_VerifySignature(digest,sizeof(digest),signature);
	if (returnCode!=STATUS_SUCCESS) {
		while(1);
	}


#endif/*Test Verification Wrapper #if*/


#endif/*Test ECDSA #if*/

while(1){

}
	return 0;
}
