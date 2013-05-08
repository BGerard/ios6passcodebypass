all: sender

%.o: %.c afc.h output.h threads.h
	gcc -mmacosx-version-min=10.4 -std=gnu99 -arch i386  -m32 -DTESTING -c -o $@ $<

sender: sender.o afc.o  
	
	gcc -mmacosx-version-min=10.4 -arch i386  -m32 -w -o sender sender.o afc.o  -F/System/Library/PrivateFrameworks/ -framework MobileDevice -framework CoreFoundation -framework Foundation -framework DeviceLink -lz

clean:
	rm -rf sender *.o 
