#include "tbb/pipeline.h"
#include "tbb/tick_count.h"
#include "tbb/task_scheduler_init.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

using namespace std;

class Buffer {
    static const size_t buffer_size = 10000;
    char* my_end;
    char storage[1+buffer_size];
public:
    char* begin() {return storage+1;}
    const char* begin() const {return storage+1;}
    char* end() const {return my_end;}
    void set_end( char* new_ptr ) {my_end=new_ptr;}
    size_t max_size() const {return buffer_size;}
    size_t size() const {return my_end-begin();}
};

//######################################
class InputFilter: public tbb::filter {
public:
    static const size_t n_buffer = 8;
    InputFilter( FILE* input_file_ );
private:
    FILE* input_file;
    size_t next_buffer;
    char last_char_of_previous_buffer;
    Buffer buffer[n_buffer];
    /*override*/ void* operator()(void*);
};

InputFilter::InputFilter( FILE* input_file_ ) : 
    filter(/*is_serial=*/true),
    next_buffer(0),
    input_file(input_file_),
    last_char_of_previous_buffer(' ')
{ 
}

void* InputFilter::operator()(void*) {
    Buffer& b = buffer[next_buffer];
    next_buffer = (next_buffer+1) % n_buffer;
    size_t n = fread( b.begin(), 1, b.max_size(), input_file );
    if( !n ) {
        // end of file
        return NULL;
    } else {
        b.begin()[-1] = last_char_of_previous_buffer;
        last_char_of_previous_buffer = b.begin()[n-1];
        b.set_end( b.begin()+n );
        return &b;
    }
}

//######################################
//! Filter that changes the first letter of each word from lower case to upper case.
class TransformFilter: public tbb::filter {
public:
    TransformFilter();
    /*override*/void* operator()( void* item );
};

TransformFilter::TransformFilter() : 
    tbb::filter(/*ordered=*/false) 
{}  

/*override*/void* TransformFilter::operator()( void* item ) {
    Buffer& b = *static_cast<Buffer*>(item);
    int prev_char_is_space = b.begin()[-1]==' ';
    for( char* s=b.begin(); s!=b.end(); ++s ) {
        if( prev_char_is_space && islower(*s) )
            *s = toupper(*s);
        prev_char_is_space = isspace((unsigned char)*s);
    }
    return &b;  
}
         
//######################################
//! Filter that writes each buffer to a file.
class OutputFilter: public tbb::filter {
    FILE* my_output_file;
public:
    OutputFilter( FILE* output_file );
    /*override*/void* operator()( void* item );
};

OutputFilter::OutputFilter( FILE* output_file ) : 
    tbb::filter(/*is_serial=*/true),
    my_output_file(output_file)
{
}

void* OutputFilter::operator()( void* item ) {
    Buffer& b = *static_cast<Buffer*>(item);
    fwrite( b.begin(), 1, b.size(), my_output_file );
    return NULL;
}

static int NThread = tbb::task_scheduler_init::automatic;
static const char* InputFileName = "input.txt";
static const char* OutputFileName = "output.txt";
static bool is_number_of_threads_set = false;

void Usage()
{
    fprintf( stderr, "Usage:\ttext_filter [input-file [output-file [nthread]]]\n");
    exit(1);
}

void ParseCommandLine(  int argc, char* argv[] ) {
    // Parse command line
    if( argc> 4 ) Usage();
    if( argc>=2 ) InputFileName = argv[1];
    if( argc>=3 ) OutputFileName = argv[2];
    if( argc>=4 ) {
        NThread = strtol(argv[3],0,0);
        if( NThread<1 ) {
            fprintf(stderr,"nthread set to %d, but must be at least 1\n",NThread);
            exit(1);
        }
        is_number_of_threads_set = true; //Number of threads is set explicitly
    }
}

void run_pipeline( int nthreads )
{
    FILE* input_file = fopen(InputFileName,"r");
    if( !input_file ) {
        perror( InputFileName );
        Usage();
    }
    FILE* output_file = fopen(OutputFileName,"w");
    if( !output_file ) {
        perror( OutputFileName );
        exit(1);
    }

    // Create the pipeline
    tbb::pipeline pipeline;

    InputFilter input_filter( input_file );
    TransformFilter transform_filter; 
    OutputFilter output_filter( output_file );

    pipeline.add_filter( input_filter );
    pipeline.add_filter( transform_filter );
    pipeline.add_filter( output_filter );

    // Run the pipeline
    tbb::tick_count t0 = tbb::tick_count::now();
    pipeline.run( InputFilter::n_buffer );
    tbb::tick_count t1 = tbb::tick_count::now();

    // Remove filters from pipeline before they are implicitly destroyed.
    pipeline.clear(); 

    fclose( output_file );
    fclose( input_file );

    if (is_number_of_threads_set) {
        printf("threads = %d time = %g\n", nthreads, (t1-t0).seconds());
    } else {
        if ( nthreads == 1 ){
            printf("serial run   time = %g\n", (t1-t0).seconds());
        } else {
            printf("parallel run time = %g\n", (t1-t0).seconds());
        }
    }
}

int main( int argc, char* argv[] ) {
    ParseCommandLine( argc, argv );
    if (is_number_of_threads_set) {
        // Start task scheduler
        tbb::task_scheduler_init init( NThread );
        run_pipeline (NThread);
    } else { // Number of threads wasn't set explicitly. Run serial and parallel version
        { // serial run
            tbb::task_scheduler_init init_serial(1);
            run_pipeline (1);
        }
        { // parallel run (number of threads is selected automatically)
            tbb::task_scheduler_init init_parallel;
            run_pipeline (0);
        }
    }
}
