win:
#	Windows
	mkdir -p release
	mkdir -p release\windows
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
	mkdir -p release/linux
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

clean:
	@echo "======================================================"
	@echo "  Clean the productions that produced by building..."
	@echo "  清理构建产物中..."
	@echo "======================================================"
	@if [ -d "release" ]; then \
		echo "  移除 release 目录..."; \
		rm -rf release; \
		echo "  release 目录已删除"; \
	else \
		echo "   release 目录不存在，无需清理"; \
	fi
	@echo "======================================================"
	@echo "  清理完成！"

.PHONY: all win linux xj380 clean