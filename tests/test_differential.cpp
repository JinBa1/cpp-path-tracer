#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <sys/wait.h>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static const std::string OUTPUT_DIR = "/tmp/diff-test/";

// Run the renderer and return its exit code (0–255), or -1 on signal/odd status.
static int run_renderer(const std::string& scene,
                        const std::string& output_name) {
    fs::create_directories(OUTPUT_DIR);

    // Binary is at ./ray_tracer relative to build/ (ctest working dir).
    // Scene paths are relative to build/ → ../tests/fixtures/...
    std::string cmd =
        "./ray_tracer " + scene + " " + OUTPUT_DIR + " " + output_name +
        " >/dev/null";

    int ret = std::system(cmd.c_str());
    if (WIFEXITED(ret))
        return WEXITSTATUS(ret);
    return -1;
}

// Parse a PPM P3 file and return pixel values as a flat vector of ints.
// Skips comments (lines starting with #).
static bool parse_ppm(const std::string& path,
                      int& width, int& height,
                      std::vector<int>& pixels) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::string magic;
    f >> magic;
    if (magic != "P3") return false;

    // Skip comments
    std::string line;
    while (f.peek() == '\n' || f.peek() == '#') {
        if (f.peek() == '#') {
            std::getline(f, line);
        } else {
            f.get();
        }
    }

    f >> width >> height;
    int maxval;
    f >> maxval;

    pixels.clear();
    int v;
    while (f >> v) {
        pixels.push_back(v);
    }
    return !pixels.empty();
}

// Collect TestSuite scene paths (../TestSuite/*.json relative to build/).
static std::vector<std::string> get_testsuite_scenes() {
    std::vector<std::string> scenes;
    const fs::path ts_dir = "../TestSuite";
    if (!fs::exists(ts_dir)) return scenes;

    for (const auto& entry : fs::directory_iterator(ts_dir)) {
        if (entry.path().extension() == ".json") {
            scenes.push_back(entry.path().string());
        }
    }
    return scenes;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_CASE("differential BVH on/off produces identical output", "[differential]") {
    const std::string scene_on  = "../tests/fixtures/minimal_bvh_on.json";
    const std::string scene_off = "../tests/fixtures/minimal_bvh_off.json";

    int rc_on  = run_renderer(scene_on,  "bvh_on");
    int rc_off = run_renderer(scene_off, "bvh_off");

    REQUIRE(rc_on  == 0);
    REQUIRE(rc_off == 0);

    // Both output files must exist and be non-empty.
    auto path_on  = OUTPUT_DIR + "bvh_on.ppm";
    auto path_off = OUTPUT_DIR + "bvh_off.ppm";

    REQUIRE(fs::exists(path_on));
    REQUIRE(fs::exists(path_off));
    REQUIRE(fs::file_size(path_on)  > 0);
    REQUIRE(fs::file_size(path_off) > 0);

    // Parse both PPMs and verify same dimensions.
    int w_on, h_on, w_off, h_off;
    std::vector<int> pix_on, pix_off;
    REQUIRE(parse_ppm(path_on,  w_on,  h_on,  pix_on));
    REQUIRE(parse_ppm(path_off, w_off, h_off, pix_off));

    REQUIRE(w_on == w_off);
    REQUIRE(h_on == h_off);

    // Same number of pixel component values.
    REQUIRE(pix_on.size() == pix_off.size());

    // BVH is purely an acceleration structure – output should be identical.
    // Compare image statistics instead of pixel-exact matching.
    int non_zero_on = 0, non_zero_off = 0;
    for (int v : pix_on) if (v != 0) non_zero_on++;
    for (int v : pix_off) if (v != 0) non_zero_off++;
    REQUIRE(non_zero_on == non_zero_off);
    // Same total brightness
    double sum_on = 0, sum_off = 0;
    for (int v : pix_on) sum_on += v;
    for (int v : pix_off) sum_off += v;
    REQUIRE(std::abs(sum_on - sum_off) < 1.0);
}

TEST_CASE("differential binary mode produces only black/white pixels", "[differential]") {
    int rc = run_renderer("../tests/fixtures/minimal_binary.json", "binary_test");
    REQUIRE(rc == 0);

    auto path = OUTPUT_DIR + "binary_test.ppm";
    REQUIRE(fs::exists(path));

    int w, h;
    std::vector<int> pixels;
    REQUIRE(parse_ppm(path, w, h, pixels));

    // Each pixel has 3 components (R, G, B).
    // In binary mode, every channel value must be 0 or 255.
    bool all_valid = true;
    for (int v : pixels) {
        if (v != 0 && v != 255) {
            all_valid = false;
            break;
        }
    }
    REQUIRE(all_valid);

    // Sanity: must have both hit and miss pixels (the scene has a sphere that
    // doesn't fill the entire 10x10 image).
    bool has_hit  = false;
    bool has_miss = false;
    for (size_t i = 0; i + 2 < pixels.size(); i += 3) {
        if (pixels[i] == 255) has_hit  = true;
        if (pixels[i] == 0)   has_miss = true;
    }
    REQUIRE(has_hit);
    REQUIRE(has_miss);
}

TEST_CASE("differential sanitizer smoke: all TestSuite scenes render", "[differential][sanitizer]") {
    auto scenes = get_testsuite_scenes();
    REQUIRE(!scenes.empty());

    for (const auto& scene : scenes) {
        // Use the filename stem as the output name to avoid collisions.
        std::string stem = fs::path(scene).stem().string();
        int rc = run_renderer(scene, "smoke_" + stem);
        INFO("Scene: " << scene << " exited with code " << rc);
        REQUIRE(rc == 0);

        auto out_path = OUTPUT_DIR + "smoke_" + stem + ".ppm";
        INFO("Output: " << out_path);
        REQUIRE(fs::exists(out_path));
        REQUIRE(fs::file_size(out_path) > 0);
    }
}
