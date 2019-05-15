let
  moz_overlay = import (builtins.fetchTarball https://github.com/mozilla/nixpkgs-mozilla/archive/master.tar.gz);
  nixpkgs = import <nixpkgs> { overlays = [ moz_overlay ]; };
  myRust = nixpkgs.latest.rustChannels.stable.rust.override {
    extensions = [
      "rust-src"
      "rls-preview"
      "clippy-preview"
      "rustfmt-preview"
    ];
  };
in
with nixpkgs;
stdenv.mkDerivation {
  name = "wordsmith-shell";
  buildInputs = [
    myRust
  ];
  RUST_SRC_PATH = "${myRust}/lib/rustlib/src";
}
