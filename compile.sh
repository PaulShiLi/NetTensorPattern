# Check if gcc is installed
if ! [ -x "$(command -v gcc)" ]; then
  echo 'Error: gcc is not installed.' >&2
  exit 1
fi

echo "Building Cython files"

python build_lib.py build_ext --inplace;