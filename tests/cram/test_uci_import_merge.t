check that uci import with mergeing is producing expected results:

  $ [ -n "$BUILD_BIN_DIR" ] && export PATH="$BUILD_BIN_DIR:$PATH"
  $ export CONFIG_DIR="$TESTDIR/config"
  $ export FUZZ_CORPUS="$TESTDIR/../fuzz/corpus"

  $ for file in $(LC_ALL=C find $FUZZ_CORPUS -type f | sort ); do
  >   valgrind --quiet --leak-check=full uci import network -m -c $CONFIG_DIR -f $file; \
  > done
  uci: Parse error (package without name) at line 0, byte 68
  uci: Parse error (invalid command) at line 0, byte 0
  uci: Parse error (invalid command) at line 1, byte 18
