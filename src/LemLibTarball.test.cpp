#include "pros/apix.h"
#include "lemlib-tarball/api.hpp"
#include "utest.h"

ASSET(my_lemlib_tarball_file_txt);

class DecoderStub : public lemlib_tarball::Decoder {
    public:
        DecoderStub(const asset& tarball);

        std::vector<std::string> getPaths();

        std::vector<asset> getAssets();

        std::string getPathName(size_t index);

        std::string getAssetContent(size_t index);
};

DecoderStub::DecoderStub(const asset& tarball)
    : Decoder(tarball) {}

std::vector<std::string> DecoderStub::getPaths() { return paths; }

std::vector<asset> DecoderStub::getAssets() { return assets; }

std::string DecoderStub::getPathName(size_t index) { return paths[index]; }

std::string DecoderStub::getAssetContent(size_t index) {
    return std::string((char*)assets[index].buf, assets[index].size);
}

static asset mockAsset(std::string content) { return {(uint8_t*)content.c_str(), content.size()}; }

static DecoderStub mockDecoder(std::string content) { return DecoderStub(mockAsset(content)); }

static std::string toString(const asset& asset) { return std::string((char*)asset.buf, asset.size); }

UTEST(decoder, handle_invalid_content) {
    ASSERT_TRUE(mockDecoder("").getPaths().size() == 0);
    ASSERT_TRUE(mockDecoder("\r").getPaths().size() == 0);
    ASSERT_TRUE(mockDecoder("\n").getPaths().size() == 0);
    ASSERT_TRUE(mockDecoder("\r\n").getPaths().size() == 0);
    ASSERT_TRUE(mockDecoder("").getPaths().size() == 0);

    // 'T' should be followed by a ' '
    ASSERT_TRUE(mockDecoder("#PATH-POINTS-START").getPaths().size() == 0);

    // 'T' should be followed by a ' '
    ASSERT_TRUE(mockDecoder("#PATH-POINTS-START\n").getPaths().size() == 0);

    // The last line
    ASSERT_TRUE(mockDecoder("#PATH-POINTS-START Path 1").getPaths().size() == 0);

    // Double '#'
    ASSERT_TRUE(mockDecoder("##PATH-POINTS-START\n").getPaths().size() == 0);

    // 'T' should be followed by a ' '
    ASSERT_TRUE(mockDecoder("#\n#PATH-POINTS-START\n#").getPaths().size() == 0);

    // ' ' space at the beginning
    ASSERT_TRUE(mockDecoder("#\n #PATH-POINTS-START \n#").getPaths().size() == 0);
}

UTEST(Decoder, handle_valid_content_1) {
    DecoderStub decoder1 = mockDecoder("#PATH-POINTS-START \n");
    ASSERT_TRUE(decoder1.getPaths().size() == 1);
    ASSERT_TRUE(decoder1.getPathName(0) == "");
    ASSERT_TRUE(decoder1.getAssetContent(0) == "");

    DecoderStub decoder2 = mockDecoder("#PATH-POINTS-START \n#");
    ASSERT_TRUE(decoder2.getPaths().size() == 1);
    ASSERT_TRUE(decoder2.getPathName(0) == "");
    ASSERT_TRUE(decoder2.getAssetContent(0) == "");

    DecoderStub decoder3 = mockDecoder("#PATH-POINTS-START \n#\n");
    ASSERT_TRUE(decoder3.getPaths().size() == 1);
    ASSERT_TRUE(decoder3.getPathName(0) == "");
    ASSERT_TRUE(decoder3.getAssetContent(0) == "");

    DecoderStub decoder4 = mockDecoder("#PATH-POINTS-START \n #\n");
    ASSERT_TRUE(decoder4.getPaths().size() == 1);
    ASSERT_TRUE(decoder4.getPathName(0) == "");
    ASSERT_TRUE(decoder4.getAssetContent(0) == " #\n");

    DecoderStub decoder5 = mockDecoder("#PATH-POINTS-START path name with spaces\ncontent\n#SOMETHING\n");
    ASSERT_TRUE(decoder5.getPaths().size() == 1);
    ASSERT_TRUE(decoder5.getPathName(0) == "path name with spaces");
    ASSERT_TRUE(decoder5.getAssetContent(0) == "content\n");

    DecoderStub decoder6 = mockDecoder("#PATH-POINTS-START path1\ncontent1\n#PATH-POINTS-START path2\ncontent2\n#END");
    ASSERT_TRUE(decoder6.getPaths().size() == 2);
    ASSERT_TRUE(decoder6.getPathName(0) == "path1");
    ASSERT_TRUE(decoder6.getAssetContent(0) == "content1\n");
    ASSERT_TRUE(decoder6.getPathName(1) == "path2");
    ASSERT_TRUE(decoder6.getAssetContent(1) == "content2\n");
}

UTEST(Decoder, handle_valid_content_2) {
    DecoderStub decoder7 = mockDecoder("#PATH-POINTS-START path1\ncontent1\n#PATH-POINTS-START path2\ncontent2");
    ASSERT_TRUE(decoder7.getPaths().size() == 2);
    ASSERT_TRUE(decoder7.getPathName(0) == "path1");
    ASSERT_TRUE(decoder7.getAssetContent(0) == "content1\n");
    ASSERT_TRUE(decoder7.getPathName(1) == "path2");
    ASSERT_TRUE(decoder7.getAssetContent(1) == "content2");

    DecoderStub decoder8(my_lemlib_tarball_file_txt);
    ASSERT_TRUE(decoder8.getPaths().size() == 2);

    DecoderStub decoder9 = mockDecoder("#PATH-POINTS-START path1\ncontent1\n");
    ASSERT_TRUE(decoder9.getPaths().size() == 1);
    ASSERT_TRUE(decoder9.getPathName(0) == "path1");
    ASSERT_TRUE(decoder9.getAssetContent(0) == "content1\n");
}

UTEST(Decoder, has) {
    DecoderStub decoder(my_lemlib_tarball_file_txt);

    ASSERT_TRUE(decoder.has("Path 1"));
    ASSERT_TRUE(decoder.has("Path 2"));

    ASSERT_FALSE(decoder.has("Path 3"));
    ASSERT_FALSE(decoder.has(" "));
    ASSERT_FALSE(decoder.has(" Path 1"));
    ASSERT_FALSE(decoder.has("Path 2 "));
    ASSERT_FALSE(decoder.has("Path 2\n"));
}

UTEST(Decoder, get) {
    DecoderStub decoder1 = mockDecoder("#PATH-POINTS-START path1\ncontent1\n#PATH-POINTS-START path2\ncontent2");
    ASSERT_TRUE(toString(decoder1.get("path1")) == "content1\n");
    ASSERT_TRUE(toString(decoder1.get("path2")) == "content2");
    ASSERT_TRUE(decoder1.get("path1").buf == decoder1["path1"].buf);
    ASSERT_TRUE(decoder1.get("path1").buf != decoder1["path2"].buf);
    // [LemLib] ERROR: Path not found: does not exist
    ASSERT_TRUE(decoder1.get("does not exist").buf == nullptr);
    // [LemLib] ERROR: Path not found: does not exist
    ASSERT_TRUE(decoder1.get("does not exist").size == 0);
}
