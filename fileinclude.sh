#! /bin/bash

base="$1"
shift
code="$base.cpp"
header="$base.h"
begin='R"#+QUOTE_BLOCK+#('
end=')#+QUOTE_BLOCK+#"'

cat << ! > "$header"
#include <string>
#include <unordered_map>
namespace fileinclude { const std::string &loaded_file(const std::string &name); }
!

cat << ! > "$code"
#include "fileinclude.h"
namespace fileinclude {
const std::string &loaded_file(const std::string &name) {
static const std::unordered_map<std::string, std::string> data{
!
for file in "$@"
	do echo "{\"$file\", $begin" >> "$code"
	cat $file >> "$code"
	echo "$end}," >> "$code"
done
cat << ! >> "$code"
};
return data.at(name);
} }
!
