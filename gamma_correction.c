#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <png.h>

double file_gamma;
int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers = NULL;

int read_png_file(char *filename);
int write_png_file(char *filename);
int gamma_correction();

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Error! Usage 'progname input_file output_file'\n");
        return -1;
    }

    if (read_png_file(argv[1])) {
        printf("Error reading file!\n");
        return -1;
    }

    if (gamma_correction()) {
        printf("Already maximum light!\n");
        return -1;
    }

    if (write_png_file(argv[2])) {
        printf("Error writing file!\n");
        return -1;
    }

    return 0;
}

int read_png_file(char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return -1;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
        return -1;

    png_infop info = png_create_info_struct(png);
    if (!info)
        return -1;

    if (setjmp(png_jmpbuf(png)))
        return -1;

    png_init_io(png, fp);

    png_read_info(png, info);

    width      = png_get_image_width(png, info);
    height     = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth  = png_get_bit_depth(png, info);
    png_get_gAMA(png, info, &file_gamma);

    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (color_type == PNG_COLOR_TYPE_RGB ||
            color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    if (row_pointers)
        return -1;

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++)
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));

    png_read_image(png, row_pointers);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);

    return 0;
}

int write_png_file(char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
        return -1;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
        return -1;

    png_infop info = png_create_info_struct(png);
    if (!info)
        return -1;

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(
                png,
                info,
                width, height,
                8,
                PNG_COLOR_TYPE_RGBA,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT
                );
    png_write_info(png, info);

    if (!row_pointers)
        return -1;

    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    for (int y = 0; y < height; y++)
        free(row_pointers[y]);

    free(row_pointers);
    fclose(fp);

    png_destroy_write_struct(&png, &info);

    return 0;
}

int gamma_correction()
{
    if (file_gamma) {
        for (int y = 0; y < height; y++) {
            png_bytep row = row_pointers[y];
            for (int x = 0; x < (width - 1); x++) {
                png_bytep px = &(row[x * 3]);
                px[0] = (png_byte)(pow(((double)px[0] / 255), file_gamma) * 255);
                px[1] = (png_byte)(pow(((double)px[1] / 255), file_gamma) * 255);
                px[2] = (png_byte)(pow(((double)px[2] / 255), file_gamma) * 255);
            }
        }
    }
    else {
        return -1;
    }

    return 0;
}
