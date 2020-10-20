all: WTF WTFserver

WTF:
	gcc WTF.c -lssl -lcrypto -o WTF 

WTFserver:
	gcc WTFserver.c -lssl -lcrypto -o WTFserver

test:
	gcc WTFtest.c -o WTFtest

clean: 
	rm -r WTFserver WTF WTFtest

