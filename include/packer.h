#pragma once
#ifndef _CPP_TEXTURE_PACKER_PACKER_
#define _CPP_TEXTURE_PACKER_PACKER_

#include <vector>
#include <tuple>
#include <algorithm>

#include "atlas.h"

namespace CppTexturePacker
{

    class Packer
    {
    private:
        unsigned int bg_color;

        unsigned int max_width;
        unsigned int max_height;

        bool enable_rotated;
        bool force_square;

        unsigned int border_padding;
        unsigned int shape_padding;
        unsigned int inner_padding;

        unsigned char trim_mode;
        bool reduce_border_artifacts;
        bool extrude;

    public:
        std::vector<Atlas> atlases;

    public:
        Packer(
            //unsigned int _bg_color=0x00000000,
            unsigned int _max_width=4096,
            unsigned int _max_height=4096,
            bool _enable_rotated=false,
            bool _force_square=false,
            unsigned int _border_padding=0,
            unsigned int _shape_padding=0,
            unsigned int _inner_padding=0,
            unsigned char _trim_mode=0
            //bool _reduce_border_arifacts=false,
            //bool _extrude=false
            ):
            //bg_color(_bg_color),
            max_width(_max_width),
            max_height(_max_height), 
            enable_rotated(_enable_rotated),
            force_square(_force_square),
            border_padding(_border_padding),
            shape_padding(_shape_padding),
            inner_padding(_inner_padding),
            trim_mode(_trim_mode)
            //reduce_border_artifacts(_reduce_border_arifacts),
            //extrude(_extrude)
        {
            atlases.emplace_back(Atlas(max_width, max_height, force_square, border_padding, shape_padding, inner_padding));
        }

        void add_image_rect(ImageRect image_rect)
        {
            unsigned int best_atlas_index = -1;
            unsigned int best_free_rect_index = -1;
            unsigned int best_rank = MAX_RANK; 
            bool best_rotated = false;

            unsigned int rank;
            unsigned int free_rect_index;
            bool rotated;

            for(int atlas_index=0; atlas_index<atlases.size(); ++atlas_index)
            {
                std::tie(rank, free_rect_index, rotated) = atlases[atlas_index].find_best_rank(image_rect, enable_rotated);

                if(rank<best_rank)
                {
                    best_atlas_index = atlas_index;
                    best_rank = rank;
                    best_free_rect_index = free_rect_index;
                    best_rotated = rotated;
                }
            }

            if(best_rank == MAX_RANK)
            {
                for(int atlas_index=0; atlas_index<atlases.size(); ++atlas_index)
                {
                    while(MAX_RANK == best_rank)
                    {
                        if(atlases[atlas_index].try_expand())
                        {
                            best_atlas_index = atlas_index;
                            std::tie(best_rank, best_free_rect_index, best_rotated) = atlases[atlas_index].find_best_rank(image_rect, enable_rotated);
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (MAX_RANK != best_rank) 
                    {
                        break;
                    }
                }

                if(best_rank == MAX_RANK)
                {
                    atlases.emplace_back(Atlas(max_width, max_height, force_square, border_padding, shape_padding, inner_padding));

                    best_atlas_index = atlases.size()-1;
                    std::tie(best_rank, best_free_rect_index, best_rotated) = atlases[best_atlas_index].find_best_rank(image_rect, enable_rotated);

                    while(MAX_RANK == best_rank)
                    {
                        if(!atlases[best_atlas_index].try_expand())
                        {
                            assert(("can not place image in max size", false));
                        }
                        
                        std::tie(best_rank, best_free_rect_index, best_rotated) = atlases[best_atlas_index].find_best_rank(image_rect, enable_rotated);
                    }

                }
            }

            if(best_rotated)
            {
                image_rect.rotate();
            }

            atlases[best_atlas_index].place_image_rect_in_free_rect(best_free_rect_index, image_rect);
        }

        void add_image_rects(std::vector<ImageRect> image_rects)
        {
            std::sort(
                image_rects.begin(), 
                image_rects.end(), 
                [](const ImageRect &a, const ImageRect &b)
                {
                    return std::max(a.width, a.height) > std::max(b.width, b.height); 
                }
                );

            for(auto image_rect: image_rects)
            {
                add_image_rect(image_rect);
            }
        }        

    };
} // namespace CppTexturePacker
#endif