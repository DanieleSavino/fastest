FASTEST_HOME="$(realpath vendor/fastest)"

name=$(basename "$PWD")

pushd "$FASTEST_HOME/python" || exit 1

pip install -r requirements.txt

export FASTEST_HOME
export FASTEST_USER_LIB="$FASTEST_HOME/demo/$name/build/release/lib$name.a"
export FASTEST_MODULE_NAME="$name"

pip install -e .

popd
