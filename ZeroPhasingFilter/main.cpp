#include <iostream>
#include <string.h>
#include <math.h>
#include <fftw3.h>
#include <map>
#include "sep_writer.h"
#include "sep_reader.h"


float get_argument ( int &   arg 
                   , int     argc
                   , char ** argv
                   )
{
    ++arg;
    if ( arg > argc )
    {
        std::cout << "parameter error" << std::endl;
        exit(1);
    }
    return atof ( argv[arg] );
}

template < typename T >
inline void mem_cpy ( T * out , T * in , int num )
{
    for ( int k(0)
        ; k < num
        ; ++k
        )
    {
        out[k] = in[k];
    }
}

int main( int argc , char ** argv )
{

    if (argc == 1)
    {
        std::cout << "wrong number of arguments" << std::endl;
        exit(1);
    }

    int arg = 0;

    arg++;

    std::string file_name( argv[arg] );

    arg++;

    std::string output_file_name( argv[arg] );

    SEPReader reader ( file_name . c_str () );
    int num_t = reader . n1;
    int num_x = reader . n2 
              * reader . n3
              * reader . n4
              * reader . n5
              * reader . n6
              * reader . n7
              * reader . n8
              ;
    std::cout << num_t << std::endl;
    std::cout << num_x << std::endl;
    if ( num_t % 2 != 0 )
    {
        std::cout << "num_t should be even" << std::endl;
        exit(1);
    }
    float * data = new float [ num_x * num_t ];
    memset ( &data[0] , 0 , num_x * num_t );
    reader . read_sepval ( & data [ 0 ] , reader . o1 , reader . o2 , reader . o3 , num_x * num_t );

    int num_f = num_t / 2 + 1;
    fftwf_complex * fdata = new fftwf_complex [ num_x * num_f ];

    float * tmp = new float [ num_t ];
    fftwf_complex * ftmp = new fftwf_complex [ num_f ];

    fftwf_plan plan ( fftwf_plan_dft_r2c_1d ( num_t
                                            , & tmp[0]
                                            , &ftmp[0]
                                            , FFTW_ESTIMATE
                                            )
                    );

    for ( int x(0)
        ; x < num_x
        ; ++x
        )
    {
        mem_cpy ( & tmp[0] , & data[x*num_t] , num_t );
        fftwf_execute ( plan );
        mem_cpy < float > ( (float*)(&fdata[x*num_f]) , (float*)(&ftmp[0]) , 2*num_f );
    }

    fftwf_destroy_plan ( plan );

    SEPWriter writer ( output_file_name . c_str () 
                     , reader . o1 , reader . d1 , 2 * num_f
                     , reader . o2 , reader . d2 , reader . n2
                     , reader . o3 , reader . d3 , reader . n3
                     , reader . get_header_labels ()
                     , reader . get_sort_order ()
                     , (output_file_name + std::string("@")) . c_str()
                     );

    writer . OpenDataFile ( (output_file_name + std::string("@")) . c_str() );

    writer . write_sepval ( (float*)fdata , reader . o1 , reader . o2 , reader . o3 , num_x * (2 * num_f) );

    return 0;

}

