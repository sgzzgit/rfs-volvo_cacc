# lib directory must be created if not already existing to hold jbus library 

include $(CAPATH_MK_DEFS)

all: lib
	make -C src 

clean:
	make -C src clean
	
lib:                                                        
	mkdir -p lib 
