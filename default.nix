{ lib, stdenv, cmake }:

let
  vcpkgFile = builtins.fromJSON (builtins.readFile ./vcpkg.json);

  pname = vcpkgFile.name;
  version = vcpkgFile.version;
in
  stdenv.mkDerivation
  {
    inherit pname;

    name = "${pname}-${version}";
    inherit version;

    src = lib.cleanSource ./.;

    nativeBuildInputs = [ cmake ];
  }
