void otpmd5init (struct otpmdx_ctx *context);void otpmd5update (struct otpmdx_ctx *context, unsigned char *input, unsigned short inputLen);void otpmd5final (unsigned char *digest, struct otpmdx_ctx *context);