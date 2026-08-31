{
  description = "Tablo Query Language";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";

    tablog = {
      url = "github:Sobottasgithub/tablog";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      tablog,
    }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };

      version = "0.0.1";

      libtablog = tablog.packages.${system}.lib;

      commonDeps = with pkgs; [
        cmake
        gcc
        gnumake
        libtablog
        arrow-cpp
        readline
        pkg-config
      ];

      mkTQLPackage =
        {
          pname,
          buildTarget,
          enableLib ? false,
          enableTest ? false,
          extraInputs ? [ ],
        }:
        pkgs.stdenv.mkDerivation {
          inherit pname version;
          src = ./.;

          buildInputs = commonDeps ++ extraInputs;

          configurePhase = ''
            cmake -B build -S $src \
              -DCMAKE_BUILD_TYPE=Release \
              -DDEF_TQL=${if enableLib then "ON" else "OFF"} \
              -DDEF_TEST=${if enableTest then "ON" else "OFF"}
          '';

          buildPhase = ''
            cmake --build build \
              --target ${buildTarget} \
              -j$NIX_BUILD_CORES
          '';

          installPhase = ''
            cmake --install build --prefix=$out
            cp LICENSE $out/
          '';
        };

    in
    {
      packages.${system} =
        let
          lib = mkTQLPackage {
            pname = "libtql";
            buildTarget = "tql";
            enableLib = true;
          };
        in
        {
          inherit lib libtablog;

          test = mkTQLPackage {
            pname = "tql-test";
            buildTarget = "tql-test";
            enableTest = true;
            extraInputs = [ lib ];
          };

          full = mkTQLPackage {
            pname = "libtql-full";
            buildTarget = "all";
            enableLib = true;
            enableTest = true;
          };

          default = self.packages.${system}.lib;
        };

      devShells.${system}.default =
        let
          devPackages = (with pkgs; [
            bridge-utils
            clang-tools

            man-db
            man-pages
            man-pages-posix
            stdman
          ]);
        in
        pkgs.mkShell {
        packages = commonDeps ++ devPackages;

        extraOutputsToInstall = [ "man" "doc" ];

        shellHook = ''
          export MANPATH="${pkgs.man-pages}/share/man:${pkgs.man-pages-posix}/share/man:$MANPATH"
          git status
        '';
      };
    };
}
