#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

#include <string>
#include <vector>
#include "util/Radiance.h"

class ImageWriter {
public:
    // Constructor takes pixel data by const reference to avoid copying
    ImageWriter(const std::vector<Radiance>& pixelData, int width, int height, const std::string& filename);

    bool writePPM() const;

private:
    const std::vector<Radiance> pixelData;
    int width;
    int height;
    std::string filename;

    std::string getPPMHeader() const;
    bool validateParameters() const;
};

#endif // IMAGEWRITER_H