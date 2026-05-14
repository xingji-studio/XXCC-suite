win:
#	Windows
	mkdir release
	mkdir release\windows
	g++ -D XXCC_GUI -D XXCC_EPF -D XXCC_CPP src/main.cpp -o release/windows/xxcc.exe
	g++ -D XXCC_GUI -D XXCC_EPF -D XXCC_C   src/main.cpp -o release/windows/xxcc--.exe
	g++ -D XXCC_GUI -D XXCC_ELF -D XXCC_CPP src/main.cpp -o release/windows/xxcc-elf.exe
	g++ -D XXCC_GUI -D XXCC_ELF -D XXCC_C   src/main.cpp -o release/windows/xxcc-elf--.exe
	g++ -D XXCC_TUI -D XXCC_EPF -D XXCC_CPP src/main.cpp -o release/windows/xxccte.exe
	g++ -D XXCC_TUI -D XXCC_EPF -D XXCC_C   src/main.cpp -o release/windows/xxccte--.exe
	g++ -D XXCC_TUI -D XXCC_ELF -D XXCC_CPP src/main.cpp -o release/windows/xxccte-elf.exe
	g++ -D XXCC_TUI -D XXCC_ELF -D XXCC_C   src/main.cpp -o release/windows/xxccte-elf--.exe

linux:
#	Linux
	mkdir release/linux
	g++ -D XXCC_GUI -D XXCC_EPF -D XXCC_CPP src/main.cpp -o release/linux/xxcc
	g++ -D XXCC_GUI -D XXCC_EPF -D XXCC_C   src/main.cpp -o release/linux/xxcc--
	g++ -D XXCC_GUI -D XXCC_ELF -D XXCC_CPP src/main.cpp -o release/linux/xxcc-elf
	g++ -D XXCC_GUI -D XXCC_ELF -D XXCC_C   src/main.cpp -o release/linux/xxcc-elf--
	g++ -D XXCC_TUI -D XXCC_EPF -D XXCC_CPP src/main.cpp -o release/linux/xxccte
	g++ -D XXCC_TUI -D XXCC_EPF -D XXCC_C   src/main.cpp -o release/linux/xxccte--
	g++ -D XXCC_TUI -D XXCC_ELF -D XXCC_CPP src/main.cpp -o release/linux/xxccte-elf
	g++ -D XXCC_TUI -D XXCC_ELF -D XXCC_C   src/main.cpp -o release/linux/xxccte-elf--

xj380:
#	XJ380
	mkdir release/xj380
	g++ -D XXCC_GUI -D XXCC_EPF -D XXCC_CPP src/main.cpp -o release/xj380/xxcc.epf
	g++ -D XXCC_GUI -D XXCC_EPF -D XXCC_C   src/main.cpp -o release/xj380/xxcc--.epf
	g++ -D XXCC_GUI -D XXCC_ELF -D XXCC_CPP src/main.cpp -o release/xj380/xxcc-elf.epf
	g++ -D XXCC_GUI -D XXCC_ELF -D XXCC_C   src/main.cpp -o release/xj380/xxcc-elf--.epf
	g++ -D XXCC_TUI -D XXCC_EPF -D XXCC_CPP src/main.cpp -o release/xj380/xxccte.epf
	g++ -D XXCC_TUI -D XXCC_EPF -D XXCC_C   src/main.cpp -o release/xj380/xxccte--.epf
	g++ -D XXCC_TUI -D XXCC_ELF -D XXCC_CPP src/main.cpp -o release/xj380/xxccte-elf.epf
	g++ -D XXCC_TUI -D XXCC_ELF -D XXCC_C   src/main.cpp -o release/xj380/xxccte-elf--.epf
