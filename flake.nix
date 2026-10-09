{
  description = "Pong";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";

    flake-parts.url = "github:hercules-ci/flake-parts";
    flake-parts.inputs.nixpkgs-lib.follows = "nixpkgs";

    treefmt-nix.url = "github:numtide/treefmt-nix";
    treefmt-nix.inputs.nixpkgs.follows = "nixpkgs";

    pre-commit-hooks-nix.url = "github:cachix/pre-commit-hooks.nix";
    pre-commit-hooks-nix.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs =
    { self, ... }@inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      imports = [
        inputs."treefmt-nix".flakeModule
      ];

      perSystem =
        {
          config,
          pkgs,
          system,
          ...
        }:
        let
          # Build tools common to all phases
          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            pkg-config
          ];

          # Runtime/library dependencies
          buildInputs = with pkgs; [
            raylib
          ];

          # Development-only tools
          devInputs =
            (with pkgs; [
              clang-tools
            ])
            ++ pkgs.lib.optionals (!pkgs.stdenv.hostPlatform.isDarwin) [
              pkgs.gdb
              pkgs.perf
              pkgs.samply
            ];

          pong-cpp = pkgs.stdenv.mkDerivation {
            pname = "pong-cpp";
            version = "0.1.0";

            # Drop build artifacts and VCS metadata so the package hash does
            # not depend on whatever happens to be in the working tree.
            src = pkgs.lib.fileset.toSource {
              root = ./.;
              fileset = pkgs.lib.fileset.fileFilter (
                { name, ... }:
                !(builtins.elem name [
                  ".git"
                  "build"
                  "Build"
                  "docs"
                  "output"
                  "result"
                ])
              ) ./.;
            };

            inherit nativeBuildInputs;
            inherit buildInputs;

            cmakeFlags = [
              "-DCMAKE_BUILD_TYPE=Release"
              "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
              # The cc-wrapper picks up raylib's include/lib dirs from
              # buildInputs, but CMake needs the prefix itself to link the
              # absolute library path and emit a matching RPATH entry.
              "-DRAYLIB_ROOT=${pkgs.raylib}"
            ];

            CXXFLAGS = [ "-std=c++17" ];
          };
        in
        {
          packages.default = pong-cpp;

          checks = {
            pre-commit = inputs."pre-commit-hooks-nix".lib.${system}.run {
              src = self;
              hooks = {
                clang-format.enable = true;
                # clang-tidy is intentionally NOT a pre-commit hook: its
                # `--fix` mode misplaces FixIts on headers when run batch-wise
                # (corrupting files). It runs instead as a read-only CI check
                # against the exported compilation database (see .github/).
                cmake-format.enable = true;
                nixfmt.enable = true;
                deadnix.enable = true;
              };
            };
          };

          treefmt = {
            projectRootFile = "flake.nix";
            programs = {
              nixfmt.enable = true;
              clang-format.enable = true;
              cmake-format.enable = true;
            };
          };

          devShells.default = pkgs.mkShell {
            inherit nativeBuildInputs;
            buildInputs = buildInputs ++ devInputs;

            shellHook = ''
              ${config.checks.pre-commit.shellHook}
              export CMAKE_EXPORT_COMPILE_COMMANDS=ON
              export RAYLIB_ROOT=${pkgs.raylib}
            '';

            CXXFLAGS = "-std=c++17 -Wall -Wextra -Werror";

            hardeningDisable = [ "fortify" ];
          };
        };
    };
}
