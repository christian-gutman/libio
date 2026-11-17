rmdir /s /q build
cmake -S src/io -B build -A Win32
cmake --build build --config Release
rmdir /s /q build

git tag -f v2.6.0.0 -m "Versión v2.6.0.0"
git push origin v2.6.0.0 --force  