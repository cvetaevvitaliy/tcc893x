# !bin/sh

all :
	@echo " ! You should input with PLATFORM !"
	@echo " ex ) make tcc8920"
	@echo "      make tcc8930"
	@echo "      make tcc8925s"
	@echo "      make tcc8800"

tcc8925s :
	@$(MAKE) -C make_boot TCC_ARCH=TCC8925S TCC_PLATFORM=tcc892x
	@$(MAKE) -C make_kernel TCC_ARCH=TCC8925S TCC_PLATFORM=tcc892x

tcc8930 :
	@$(MAKE) -C make_boot TCC_ARCH=TCC893X TCC_PLATFORM=tcc893x
	@$(MAKE) -C make_kernel TCC_ARCH=TCC893X TCC_PLATFORM=tcc893x

tcc8920 :
	@$(MAKE) -C make_boot TCC_ARCH=TCC892X TCC_PLATFORM=tcc892x
	@$(MAKE) -C make_kernel TCC_ARCH=TCC892X TCC_PLATFORM=tcc892x

tcc8800 : 
	@$(MAKE) -C make_boot TCC_ARCH=TCC880X TCC_PLATFORM=tcc88xx
	@$(MAKE) -C make_kernel TCC_ARCH=TCC880X TCC_PLATFORM=tcc88xx

clean : 
	@rm -rf  make_*/*.lo make_*/*.o
	@echo "rm *lo *o"
