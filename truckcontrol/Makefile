# Top level make file for Truck Control code
# Build must be done first in can directory to create libjbus.a

all:
	make -C long_ctl/src/common
	make -C long_ctl/src/cmdtest
	make -C long_ctl/src/xyl
#	make -C long_ctl/src/template
	make -C long_ctl/src/profile
	make -C long_ctl/src/vehcomm
	make -C long_ctl/src/avcs
	make -C test

install: all
	if [[ ! -d test ]] ; then mkdir -p test; fi
	make install -C long_ctl/src/common
	make install -C long_ctl/src/cmdtest
	make install -C long_ctl/src/xyl
	make install -C long_ctl/src/profile
#	make install -C long_ctl/src/template
	make install -C long_ctl/src/vehcomm
	make install -C long_ctl/src/avcs
	make install -C test

clean:
	make clean -C long_ctl/src/common
	make clean -C long_ctl/src/cmdtest
	make clean -C long_ctl/src/xyl
	make clean -C long_ctl/src/profile
#	make clean -C long_ctl/src/template
	make clean -C long_ctl/src/vehcomm
	make clean -C long_ctl/src/avcs
	make clean -C test
