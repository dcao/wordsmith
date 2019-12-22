{
  pkgs   ? import <nixpkgs> {}
, stdenv ? pkgs.stdenv
, shell ? false
}:
stdenv.mkDerivation {
  name = "wordsmith";
  version = "0.0.1";
  src = if shell then null else ./.;
  buildInputs = with pkgs; [
    pkgconfig hyperscan
  ] ++ stdenv.lib.optionals shell ([ gdb valgrind clang lld ]);

  nativeBuildInputs = with pkgs; [
    zig
  ];
  
  installPhase = ''
      mkdir -p $out/bin
      cp ws $out/bin
  '';
}
