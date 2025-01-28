{ pkgs ? import <nixpkgs> {}, pkg ? (pkgs.callPackage ./. {}) }:


pkgs.mkShell.override { stdenv = pkg.stdenv; } {
  inputsFrom = [ pkg ];
  nativeBuildInputs = [ ];
}
