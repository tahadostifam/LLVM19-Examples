{
  description = "LLVM19 Examples flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      overlays = [ ];
      pkgs = import nixpkgs {
        inherit system overlays;
      };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        name = "dev-shell";

        buildInputs = with pkgs; [
          gcc
          libgcc
          gcc_multi
          clang-tools
          clang
          libffi
          libffi.dev
          isl
          libxml2
          flex
          bison
          zlib
          zlib.dev
          llvmPackages.llvm
          llvmPackages.lld
          llvmPackages.compiler-rt
          llvmPackages.stdenv
        ];

        shellHook = ''
          export LIBRARY_PATH="${pkgs.gcc_multi}/lib:${pkgs.libxml2}/lib:${pkgs.flex}/lib:${pkgs.bison}/lib:${pkgs.zlib}/lib:$LIBRARY_PATH"
          export LD_LIBRARY_PATH="${pkgs.gcc_multi}/lib:${pkgs.libxml2}/lib:${pkgs.flex}/lib:${pkgs.bison}/lib:${pkgs.zlib}/lib:$LIBRARY_PATH"
          export ZLIB_LIBRARY="${pkgs.zlib}/lib"
          export ZLIB_INCLUDE_DIR="${pkgs.zlib.dev}/include"
          export CMAKE_PREFIX_PATH="${pkgs.zlib.dev}:${pkgs.zlib}:${pkgs.libffi.dev}:${pkgs.libffi}:${pkgs.libxml2}:${pkgs.llvmPackages.llvm}:${pkgs.llvmPackages.lld}:${pkgs.llvmPackages.compiler-rt}"
        '';
      };
    };
}
