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
      pkgconfig hyperscan tinycc
  ] ++ stdenv.lib.optionals shell ([]);
  
  installPhase = ''
      mkdir -p $out/bin
      cp wordsmith $out/bin
  '';
}
