HOME=.
!include $(HOME)\rules.wat

all: core drivers sarien

core: .symbolic
	@echo Building core engine
	cd src\core
	wmake -h -f makefile.wat HOME=..\..

drivers: .symbolic
	@echo Building master graphics driver
	cd src\graphics\msdos
	wmake -h -f makefile.wat HOME=..\..\..
	@echo Building filesys driver
	cd src\filesys\msdos
	wmake -h -f makefile.wat HOME=..\..\..
	cd ..\..\..
	@echo Building sound driver
	cd src\sound\dummy
	wmake -h -f makefile.wat HOME=..\..\..
	cd ..\..\..

clean: .symbolic
	@if exist *.bak @del *.bak
	@if exist bin\*.exe @del bin\*.exe
	@echo Cleaning core
	cd src\core
	wmake -h -f makefile.wat clean HOME=..\..
	@echo Cleaning msdos code
	cd src\graphics\msdos
	wmake -h -f makefile.wat clean HOME=..\..\..
	@echo Cleaning sound driver
	cd src\sound\dummy
	wmake -h -f makefile.wat clean HOME=..\..\..
	cd ..\..\..
	@echo Cleaning filegsys
	cd src\filesys\msdos
	wmake -h -f makefile.wat clean HOME=..\..\..
	cd ..\..\..
	@echo Cleaning Libs
	cd lib
	@del *.lib
	cd ..

sarien: .symbolic
	@echo Creating binary
	%create test.lnk
	%append test.lnk system $(sys)
	%append test.lnk $(lflags)
	%append test.lnk name bin\sarien.exe
	%append test.lnk library lib\agi_core.lib
	%append test.lnk library lib\driver.lib
	%append test.lnk library lib\sound.lib
	%append test.lnk library lib\fileglob.lib
	%append test.lnk file src\core\main.obj
	wlink @test.lnk

.error: .symbolic
	@echo.
	@echo An Error Occured
	@if exist test.lnk @del test.lnk

.after: .symbolic
	@if exist test.lnk @del test.lnk

tarball: .symbolic
	wmake -h -f makefile.wat clean
	cd ..
	tar cvfz sarien-$(VERSION).tar.gz sarien-$(VERSION)
