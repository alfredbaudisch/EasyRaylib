package file_version_builder

import "core:os"
import "core:fmt"
import "core:strings"

dir_path_to_file_infos :: proc(path: string) -> []os.File_Info {
	d, derr := os.open(path, os.O_RDONLY)
	if derr != 0 {
		panic("open failed")
	}
	defer os.close(d)

	{
		file_info, ferr := os.fstat(d)
		defer os.file_info_delete(file_info)

		if ferr != 0 {
			panic("stat failed")
		}
		if !file_info.is_dir {
			panic("not a directory")
		}
	}

	file_infos, _ := os.read_dir(d, -1)
	return file_infos
}

main :: proc() {
	f, _ := os.open("source/file_versions.odin", os.O_WRONLY | os.O_CREATE | os.O_TRUNC, 0o644)
	defer os.close(f)

	fmt.fprintln(f, "package game")
	fmt.fprintln(f, "")
	fmt.fprintln(f, "import \"core:os\"")

	fmt.fprintln(f, "")
	fmt.fprintln(f, "FileVersion :: struct {")
	fmt.fprintln(f, "\tpath: string,")
	fmt.fprintln(f, "\tmodification_time: os.File_Time,")
	fmt.fprintln(f, "}")
	fmt.fprintln(f, "")
	fmt.fprintln(f, "file_versions := []FileVersion {")

	files_in_this_folder := dir_path_to_file_infos("source")
	for a in files_in_this_folder {
		if a.name == "file_versions.odin" {
			continue
		}

		if strings.has_suffix(a.name, ".odin") {
			mod, mod_err := os.last_write_time_by_name(a.fullpath)

			if mod_err != os.ERROR_NONE {
				continue
			}

			fmt.fprintf(f, "\t{{ path = %q, modification_time = %v }},\n", a.fullpath, mod)
		}
	}

	fmt.fprintln(f, "}")
	fmt.fprintln(f, "")
}