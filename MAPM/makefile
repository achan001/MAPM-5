CC = gcc -Wall -std=gnu99
CFLAGS = -c -O2 -fwrapv $(ARG)
MAPM_LIB = libmapm.a
DEL = rm -f

all:		$(MAPM_LIB) validate.exe
clean:;	@$(DEL) *.o

rpn:
	$(CC) rpn.c -orpn -Os -s $(MAPM_LIB)

OBJS = \
	mapm_hasn.o \
	mapm_hsin.o \
	mapm_pow.o \
	mapm_log.o \
	mapm_lg2.o \
	mapm_exp.o \
	mapm_lg3.o \
	mapm_asin.o \
	mapm_asn0.o \
	mapm_sin.o \
	mapm_cpi.o \
	mapm_sqrt.o \
	mapm_cbrt.o \
	mapm_dbl.o \
	mapm_fact.o \
	mapm_gcd.o \
	mapm_pwr2.o \
	mapm_pwr3.o \
	mapm_rnd.o \
	mapm_flr.o \
	mapm_str.o \
	mapm_rcp.o \
	mapm_stck.o \
	mapm_div.o \
	mapm_mul.o \
	mapm_add.o \
	mapm_set.o \
	mapm_utl2.o \
	mapm_utl1.o \
	mapm_cnst.o \
	mapm_fmul.o \
	mapm_util.o \
	mapm_fft.o

$(MAPM_LIB): $(OBJS)
	@$(DEL) $(MAPM_LIB)
	@ar rc $(MAPM_LIB) $(OBJS)
	@strip --strip-unneeded $(MAPM_LIB)

validate.exe: validate.o $(MAPM_LIB)
	$(CC) validate.o -s -ovalidate $(MAPM_LIB) -lm

mapm_hasn.o: mapm_hasn.c;	$(CC) $(CFLAGS) mapm_hasn.c
mapm_hsin.o: mapm_hsin.c;	$(CC) $(CFLAGS) mapm_hsin.c
mapm_pow.o: mapm_pow.c;		$(CC) $(CFLAGS) mapm_pow.c
mapm_log.o: mapm_log.c;		$(CC) $(CFLAGS) mapm_log.c
mapm_lg2.o: mapm_lg2.c;		$(CC) $(CFLAGS) mapm_lg2.c
mapm_lg3.o: mapm_lg3.c;		$(CC) $(CFLAGS) mapm_lg3.c
mapm_exp.o: mapm_exp.c;		$(CC) $(CFLAGS) mapm_exp.c
mapm_asin.o: mapm_asin.c;	$(CC) $(CFLAGS) mapm_asin.c
mapm_asn0.o: mapm_asn0.c;	$(CC) $(CFLAGS) mapm_asn0.c
mapm_sin.o: mapm_sin.c;		$(CC) $(CFLAGS) mapm_sin.c
mapm_cpi.o: mapm_cpi.c;		$(CC) $(CFLAGS) mapm_cpi.c
mapm_flr.o: mapm_flr.c;		$(CC) $(CFLAGS) mapm_flr.c
mapm_str.o: mapm_str.c;		$(CC) $(CFLAGS) mapm_str.c
mapm_gcd.o: mapm_gcd.c;		$(CC) $(CFLAGS) mapm_gcd.c
mapm_sqrt.o: mapm_sqrt.c;	$(CC) $(CFLAGS) mapm_sqrt.c
mapm_cbrt.o: mapm_cbrt.c;	$(CC) $(CFLAGS) mapm_cbrt.c
mapm_fact.o: mapm_fact.c;	$(CC) $(CFLAGS) mapm_fact.c
mapm_pwr2.o: mapm_pwr2.c;	$(CC) $(CFLAGS) mapm_pwr2.c
mapm_pwr3.o: mapm_pwr3.c;	$(CC) $(CFLAGS) mapm_pwr3.c
mapm_rnd.o: mapm_rnd.c;		$(CC) $(CFLAGS) mapm_rnd.c
mapm_stck.o: mapm_stck.c;	$(CC) $(CFLAGS) mapm_stck.c
mapm_rcp.o: mapm_rcp.c;		$(CC) -DFAST_DIV=180 $(CFLAGS) mapm_rcp.c
mapm_div.o: mapm_div.c;		$(CC) $(CFLAGS) mapm_div.c
mapm_mul.o: mapm_mul.c;		$(CC) -DFAST_MUL=90 $(CFLAGS) mapm_mul.c
mapm_add.o: mapm_add.c;		$(CC) $(CFLAGS) mapm_add.c
mapm_set.o: mapm_set.c;		$(CC) $(CFLAGS) mapm_set.c
mapm_cnst.o: mapm_cnst.c;	$(CC) $(CFLAGS) mapm_cnst.c
mapm_fmul.o: mapm_fmul.c;	$(CC) -DFFT_MUL=2 $(CFLAGS) mapm_fmul.c
mapm_utl1.o: mapm_utl1.c;	$(CC) $(CFLAGS) mapm_utl1.c
mapm_utl2.o: mapm_utl2.c;	$(CC) $(CFLAGS) mapm_utl2.c
mapm_util.o: mapm_util.c;	$(CC) $(CFLAGS) mapm_util.c
mapm_dbl.o: mapm_dbl.c;		$(CC) $(CFLAGS) -O3 mapm_dbl.c
mapm_fft.o: mapm_fft.c;		$(CC) $(CFLAGS) -O3 -ffast-math mapm_fft.c
validate.o: validate.c;		$(CC) $(CFLAGS) -Os validate.c
