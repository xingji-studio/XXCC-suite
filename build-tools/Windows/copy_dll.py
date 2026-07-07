import os
import shutil
import sys
import argparse
from pathlib import Path
import subprocess
import platform

class CygwinDLLCopier:
    def __init__(self):
        self.source_dir = None
        self.target_dir = None
        self.overwrite_mode = 'skip'  # 'skip', 'overwrite', 'ask'
        self.copied_count = 0
        self.skipped_count = 0
        self.failed_list = []
        self.overwritten_count = 0
        
    def detect_cygwin_bin(self):
        """自动检测 Cygwin 的 bin 目录"""
        possible_paths = []
        
        # 常见 Cygwin 安装路径
        common_paths = [
            r"C:\cygwin64\bin",
            r"C:\cygwin\bin",
            r"D:\cygwin64\bin",
            r"D:\cygwin\bin",
        ]
        
        # 检查常见路径
        for path in common_paths:
            if Path(path).exists():
                possible_paths.append(path)
        
        # 通过注册表检测（Windows）
        if platform.system() == "Windows":
            try:
                import winreg
                # 从注册表读取 Cygwin 安装路径
                reg_paths = [
                    r"SOFTWARE\Cygwin\setup",
                    r"SOFTWARE\Wow6432Node\Cygwin\setup",
                ]
                for reg_path in reg_paths:
                    try:
                        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, reg_path)
                        install_dir = winreg.QueryValueEx(key, "rootdir")[0]
                        if install_dir:
                            bin_path = Path(install_dir) / "bin"
                            if bin_path.exists():
                                possible_paths.append(str(bin_path))
                        winreg.CloseKey(key)
                    except:
                        pass
            except:
                pass
        
        # 通过 where 命令检测（Windows）
        if platform.system() == "Windows":
            try:
                result = subprocess.run(
                    ["where", "cygwin1.dll"], 
                    capture_output=True, 
                    text=True,
                    shell=True
                )
                if result.stdout:
                    # 取第一个找到的路径
                    first_path = result.stdout.strip().split('\n')[0]
                    bin_path = Path(first_path).parent
                    if bin_path.exists() and str(bin_path) not in possible_paths:
                        possible_paths.append(str(bin_path))
            except:
                pass
        
        # 通过环境变量检测
        cygwin_root = os.environ.get("CYGWIN_ROOT") or os.environ.get("CYGWINROOT")
        if cygwin_root:
            bin_path = Path(cygwin_root) / "bin"
            if bin_path.exists() and str(bin_path) not in possible_paths:
                possible_paths.append(str(bin_path))
        
        return possible_paths
    
    def get_user_input(self, prompt, default=None, choices=None):
        """获取用户输入"""
        if choices:
            prompt = f"{prompt} ({'/'.join(choices)})"
            if default:
                prompt = f"{prompt} [默认: {default}]"
        
        while True:
            user_input = input(f"{prompt}: ").strip()
            if not user_input and default:
                return default
            if choices:
                if user_input.lower() in [c.lower() for c in choices]:
                    # 返回原始大小写
                    for choice in choices:
                        if choice.lower() == user_input.lower():
                            return choice
            else:
                return user_input
            print(f" 无效输入，请选择: {', '.join(choices)}")
    
    def select_source_directory(self, auto_detect=True):
        """选择源目录"""
        if auto_detect:
            detected_paths = self.detect_cygwin_bin()
            
            if detected_paths:
                print("\n 检测到以下 Cygwin bin 目录：")
                for i, path in enumerate(detected_paths, 1):
                    print(f"  {i}. {path}")
                
                choice = self.get_user_input(
                    f"\n请选择源目录 (1-{len(detected_paths)}) 或输入自定义路径",
                    default="1"
                )
                
                # 检查是否是数字选择
                if choice.isdigit() and 1 <= int(choice) <= len(detected_paths):
                    self.source_dir = detected_paths[int(choice) - 1]
                    return True
                else:
                    # 作为自定义路径处理
                    self.source_dir = choice
                    if Path(self.source_dir).exists():
                        return True
                    else:
                        print(f" 路径不存在: {self.source_dir}")
                        return False
            else:
                print("  未检测到 Cygwin 安装路径")
        
        # 手动输入
        while True:
            path = self.get_user_input("请输入 Cygwin bin 目录路径", default=r"C:\cygwin64\bin")
            if Path(path).exists():
                self.source_dir = path
                return True
            print(f" 路径不存在: {path}")
            retry = self.get_user_input("是否重新输入？(y/n)", default="y", choices=["y", "n"])
            if retry.lower() == "n":
                return False
    
    def select_target_directory(self):
        """选择目标目录"""
        default_target = r"C:\Users\yydsh\Desktop\ProjectForStudio\XJ380_APPS\xact\release\windows"
        
        while True:
            path = self.get_user_input(
                f"请输入目标目录路径",
                default=default_target
            )
            # 自动创建目录
            try:
                Path(path).mkdir(parents=True, exist_ok=True)
                self.target_dir = path
                return True
            except Exception as e:
                print(f" 无法创建目录: {e}")
                retry = self.get_user_input("是否重新输入？(y/n)", default="y", choices=["y", "n"])
                if retry.lower() == "n":
                    return False
    
    def select_overwrite_mode(self):
        """选择覆盖模式"""
        print("\n 文件处理模式：")
        print("  1. 跳过已存在的文件 (默认)")
        print("  2. 覆盖所有已存在的文件")
        print("  3. 逐个询问")
        
        choice = self.get_user_input("请选择模式 (1-3)", default="1", choices=["1", "2", "3"])
        
        if choice == "1":
            self.overwrite_mode = 'skip'
            print(" 已选择: 跳过已存在的文件")
        elif choice == "2":
            self.overwrite_mode = 'overwrite'
            print(" 已选择: 覆盖所有已存在的文件")
        elif choice == "3":
            self.overwrite_mode = 'ask'
            print(" 已选择: 逐个询问")
    
    def should_overwrite(self, file_name, target_path):
        """判断是否覆盖文件"""
        if self.overwrite_mode == 'skip':
            return False
        elif self.overwrite_mode == 'overwrite':
            return True
        elif self.overwrite_mode == 'ask':
            while True:
                choice = self.get_user_input(
                    f"文件 '{file_name}' 已存在，是否覆盖？(y/n/a)",
                    choices=["y", "n", "a"]
                )
                if choice.lower() == "y":
                    return True
                elif choice.lower() == "n":
                    return False
                elif choice.lower() == "a":
                    self.overwrite_mode = 'overwrite'
                    print(" 已切换到: 覆盖所有")
                    return True
        return False
    
    def copy_dlls(self):
        """执行复制操作"""
        if not self.source_dir or not self.target_dir:
            print(" 源目录或目标目录未设置")
            return False
        
        source_path = Path(self.source_dir)
        target_path = Path(self.target_dir)
        
        if not source_path.exists():
            print(f" 源目录不存在: {self.source_dir}")
            return False
        
        # 获取所有 .dll 文件
        dll_files = list(source_path.glob("*.dll"))
        if not dll_files:
            print(f" 源目录中没有找到 .dll 文件: {self.source_dir}")
            return False
        
        print(f"\n| 源目录: {self.source_dir}")
        print(f"| 目标目录: {self.target_dir}")
        print(f"| 找到 {len(dll_files)} 个 DLL 文件")
        print("=" * 60)
        
        # 遍历复制
        for dll_file in dll_files:
            try:
                target_file = target_path / dll_file.name
                
                if target_file.exists():
                    if not self.should_overwrite(dll_file.name, target_file):
                        print(f"⏭  跳过: {dll_file.name}")
                        self.skipped_count += 1
                        continue
                    else:
                        print(f" 覆盖: {dll_file.name}")
                        self.overwritten_count += 1
                else:
                    print(f" 复制: {dll_file.name}")
                
                shutil.copy2(dll_file, target_file)
                self.copied_count += 1
                
            except Exception as e:
                print(f" 失败: {dll_file.name} - {e}")
                self.failed_list.append(dll_file.name)
        
        # 输出统计
        self.print_summary()
        return True
    
    def print_summary(self):
        """打印统计信息"""
        print("\n" + "=" * 60)
        print(" 操作完成！")
        print(f"   成功复制: {self.copied_count} 个")
        print(f"  ⏭  跳过: {self.skipped_count} 个")
        print(f"   覆盖: {self.overwritten_count} 个")
        if self.failed_list:
            print(f"   失败: {len(self.failed_list)} 个")
            print(f"     失败文件: {', '.join(self.failed_list)}")
        print("=" * 60)


def parse_arguments():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(
        description="Cygwin DLL 复制工具 - 将 Cygwin bin 目录的 DLL 复制到指定目录",
        epilog="示例:\n"
               "  python copy_dll.py\n"
               "  python copy_dll.py -s L:\\WindowsApp\\Cygwin_seem_MS2Y2\\bin\n"
               "  python copy_dll.py -s C:\\cygwin64\\bin -t C:\\my_project\\libs -o\n"
               "  copy_dll.exe -s L:\\WindowsApp\\Cygwin_seem_MS2Y2\\bin -o"
    )
    
    parser.add_argument(
        '-s', '--source',
        help='源目录路径 (Cygwin bin 目录)'
    )
    parser.add_argument(
        '-t', '--target',
        help='目标目录路径',
        default=r"C:\Users\yydsh\Desktop\ProjectForStudio\XJ380_APPS\xact\release\windows"
    )
    parser.add_argument(
        '-o', '--overwrite',
        action='store_true',
        help='覆盖已存在的文件 (默认跳过)'
    )
    parser.add_argument(
        '-a', '--ask',
        action='store_true',
        help='逐个询问是否覆盖'
    )
    parser.add_argument(
        '--no-detect',
        action='store_true',
        help='禁用自动检测 Cygwin 位置'
    )
    
    return parser.parse_args()


def main():
    """主函数"""
    # 解析命令行参数
    args = parse_arguments()
    
    # 创建复制器实例
    copier = CygwinDLLCopier()
    
    # 设置覆盖模式
    if args.overwrite:
        copier.overwrite_mode = 'overwrite'
    elif args.ask:
        copier.overwrite_mode = 'ask'
    
    # 设置目标目录
    copier.target_dir = args.target
    Path(copier.target_dir).mkdir(parents=True, exist_ok=True)
    
    # 设置源目录
    if args.source:
        # 命令行指定了源目录
        if Path(args.source).exists():
            copier.source_dir = args.source
            print(f" 使用指定的源目录: {args.source}")
        else:
            print(f" 指定的源目录不存在: {args.source}")
            sys.exit(1)
    else:
        # 交互式选择
        print("\n" + "=" * 60)
        print("  Cygwin DLL 复制工具")
        print("=" * 60)
        
        # 选择源目录
        if not copier.select_source_directory(auto_detect=not args.no_detect):
            print(" 未能选择有效的源目录")
            sys.exit(1)
        
        # 选择目标目录
        if not copier.select_target_directory():
            print(" 未能选择有效的目标目录")
            sys.exit(1)
        
        # 选择覆盖模式（如果命令行没有指定）
        if not args.overwrite and not args.ask:
            copier.select_overwrite_mode()
    
    # 执行复制
    print(f"\n 源目录: {copier.source_dir}")
    print(f" 目标目录: {copier.target_dir}")
    
    # 最终确认
    if not args.source:
        confirm = copier.get_user_input("\n确认开始复制？(y/n)", default="y", choices=["y", "n"])
        if confirm.lower() == "n":
            print(" 已取消操作")
            sys.exit(0)
    
    # 执行复制
    success = copier.copy_dlls()
    
    if not success:
        sys.exit(1)
    
    input("\n按 Enter 键退出...") if not args.source else None


if __name__ == "__main__":
    main()