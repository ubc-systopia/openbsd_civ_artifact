HELPER ?= bsdauth

.if "${HELPER}" == "bsdauth"
.include "${.CURDIR}/Makefile.bsdauth"
.elif "${HELPER}" == "file"
.include "${.CURDIR}/Makefile.file"
.else
.error "HELPER must be bsdauth or file"
.endif
