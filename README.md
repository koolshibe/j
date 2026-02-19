# LemLib Tarball

LemLib Tarball is an extension library for [LemLib](https://lemlib.readthedocs.io/) that enables handling multiple paths in a single file for VEX V5 robots. This solves the limitation of LemLib's original path file format which only supports one path per file.

## Features

- Load multiple paths from a single "LemLib Tarball" file
- Simple API to access paths by name
- Compatible with [PATH.JERRYIO](https://path.jerryio.com) path editor
- Seamless integration with LemLib's `chassis.follow()` function

## Installation

Install LemLib Tarball using PROS CLI:

```bash
pros c add-depot LemLibTarball https://raw.githubusercontent.com/Jerrylum/LemLibTarball/depot/depot.json
pros c apply LemLibTarball
```

## Usage

### Create your paths

1. Open [PATH.JERRYIO](https://path.jerryio.com)
2. Select "LemLib Tarball" format in the editor
3. Create and customize your paths
4. Save the file to your project's `static` folder

You can also read the [PATH.JERRYIO documentation](https://docs.path.jerryio.com/docs/formats/LemLibTarballFormatV0_5) for more tips and best practices.

### Reference your paths in your code

```cpp
// Import the tarball file as an asset
ASSET(my_lemlib_tarball_file_txt);

// Create a decoder for the tarball
lemlib_tarball::Decoder decoder(my_lemlib_tarball_file_txt);

void autonomous() {
  // Set initial robot pose
  chassis.setPose(0, 0, 0);
  // Follow paths by name
  chassis.follow(decoder["Path 1"], 15, 2000);
  chassis.follow(decoder["Path 2"], 15, 2000);
}
```

## Development

### Running Tests Locally

This project uses a forked version of the VEX V5 QEMU simulator to run tests locally and in GitHub Actions. Please refer to [jerrylum/vex-v5-qemu-gh-action](https://github.com/jerrylum/vex-v5-qemu-gh-action) for more details.

1. Build the VEX V5 QEMU simulator (one-time setup):

```bash
git clone https://github.com/jerrylum/vex-v5-qemu-gh-action
cd vex-v5-qemu-gh-action
docker build -f Dockerfile.local -t vex-v5-qemu .
```

2. Build and run tests:

```bash
# Build with test flag enabled
pros make all EXTRA_CXXFLAGS=-DUNIT_TEST

# Run tests in simulator
docker run --rm -it -v "./bin/monolith.bin:/test.bin" vex-v5-qemu simulator monolith /test.bin
```

### Continuous Integration

Tests are automatically run on every push and pull request using GitHub Actions. The workflow is defined in [.github/workflows/pros-build.yml](.github/workflows/pros-build.yml).

### Code Style

This project follows LemLib's C++ code style. Please refer to [LemLib's C++ Style](https://github.com/LemLib/cpp-style) for more details.

### Release Process

1. Update version number in `Makefile`
2. Create and push a new tag:

```bash
git tag v1.0.0
git push origin v1.0.0
```

3. GitHub Actions will automatically:
   - Build the template
   - Run tests
   - Generate release artifacts
4. Create a new release in the GitHub UI
   - Attach the release artifacts
   - Publish the release
5. GitHub Actions will automatically:
   - Update the PROS depot with the new release
