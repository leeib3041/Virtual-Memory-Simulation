#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

#define PAGE_TABLE_SIZE 65536 // 64 Ki

class TLB_Entry
{
 private:
    unsigned char valid;      /* could be single bit */
    unsigned short vpn;       /* 16 bits */
    unsigned char pfn;        /* 8 bits */

 public:
    TLB_Entry(): valid(0x00), vpn(0x0000), pfn(0x00) {}
    ~TLB_Entry() {}
    // getters
    unsigned int get_valid() { return valid; }
    unsigned short get_vpn() { return vpn; }
    unsigned char get_pfn() { return pfn; }
    // check/update functions
    bool hit( unsigned short vpage)
    {
        if( valid == 0x00 ) return false;
        else {
            if( vpage == vpn ) return true;
            return false;
        }
    }
    void update( unsigned short vp, unsigned char p)
    {
        valid = 0x01;
        vpn = vp;
        pfn = p;
    }
};

class Page_Table_Entry
{
 private:
    unsigned char presence;   /* could be single bit */
    unsigned char pfn;        /* 8 bits */

 public:
    Page_Table_Entry(): presence(0x00), pfn(0x00) {}
    ~Page_Table_Entry() {}
    // getters
    unsigned int get_presence() { return presence; }
    unsigned char get_pfn() { return pfn; }
    // check/update/invalidate functions
    bool hit()
    {
        if( presence == 0x01 ) return true;
        return false;
    }
    void update( int p ) 
    {
        presence = 0x01;
        pfn = p;
    }
    void invalidate()
    {
        presence = 0x00;
        pfn = 0x00;
    }
};

class Core_Map_Entry
{
 private:
    unsigned char valid;      /* could be single bit */
    unsigned char use_vector; /* 8 bits for pseudo-LRU replacement */
    unsigned short vpn;       /* 16 bits */

 public:
    Core_Map_Entry(): valid(0x00), use_vector(0x00), vpn(0x0000) {}
    ~Core_Map_Entry() {}
    // setter/shift
    void set_useVector() { use_vector += 0x80; }
    void shift_useVector() { use_vector = use_vector >> 1; }
    // getters
    unsigned char get_valid() { return valid; }
    unsigned char get_useVector() { return use_vector; }
    unsigned short get_vpn() { return vpn; }
    // check/update functions
    bool free()
    {
        if( valid == 0x00 ) return true;
        return false;
    }
    void update( unsigned short v )
    {
        valid = 0x01;
        use_vector = 0x80;
        vpn = v;
    }
};
// function prototypes
void simulate( int, int, int, int );
void print_Statistic( int, int, int );
void print_Verbose( int, int, TLB_Entry*, Core_Map_Entry*, Page_Table_Entry* );

int main( int argc, char *argv[] )
{
    // command line argument checks
    if( argc > 2 ) {
        cout << "Too many arguments" << endl;
        return 0;
    }
    if( argc == 2 ) {
        string verbose = argv[1];
        if( verbose != "-v" ) {
            cout << "Did you mean -v?" << endl;
            return 0;
        }
    }
    // reading sample configuration file
    string garbage;
    int num_PF = 0, num_TE = 0, num_UP = 0;
    ifstream config( "paging.cfg");
    if( config.is_open() ) {
        while( config >> garbage ) {
            if( garbage == "PF" ) config >> num_PF;
            else if ( garbage == "TE" ) config  >> num_TE;
            else if( garbage == "UP" ) config >> num_UP;
            else cout << "unknown parameter" << endl;
        }
        config.close();
    } else cout << "Unable to locate paging.cfg. P"
                << "lease make sure paging.cfg is in the same directory" << endl;
    // start program
    simulate( argc, num_PF, num_TE, num_UP );
    return 0;
}

void simulate( int argc, int num_PF, int num_TE, int num_UP )
{ 
    TLB_Entry TLBE[num_TE];
    Core_Map_Entry CME[num_PF];
    Page_Table_Entry PTE[PAGE_TABLE_SIZE];
    int FIFO = 0; // fully associative FIFO policy for TLB
    string virtual_address;
    int access_count = 1, tlb_miss_count = 0, page_fault_count = 0, i = 0;
    unsigned short physical_address;

    if( argc == 2 ) {
        cout << endl <<  "paging simulation" << endl
             << "  " << PAGE_TABLE_SIZE 
                     << " virtual pages in the virtual address space" << endl
             << "  " << num_PF 
                     << " physical page frames" << endl
             << "  " << num_TE 
                     << " TLB entries" << endl
             << "  use vectors in core map are shifted every " 
                     << num_UP << " accesses" << endl << endl;
    }
    while( cin >> virtual_address ) {
        if( virtual_address.size() > 6 ) {
            print_Statistic( access_count, tlb_miss_count, page_fault_count );
            print_Verbose( num_TE, num_PF, TLBE, CME, PTE );
        }
        else {
            unsigned int VA = std::stoi( virtual_address, nullptr, 16 ); // convert string to int
            bool hit = false;
            unsigned short vPage = VA >> 8;
            unsigned char offset = VA & 0xff;

            // start access print if -v
            if( argc == 2 ) {
                cout << "access " << dec << access_count << ":" << endl
                     << "  virtual address is              0x" << virtual_address << endl;
            }
            // check TLB table
            if( FIFO == ( num_TE ) ) FIFO = 0;
            for( i = 0; i < num_TE; i++ ) {
                if( TLBE[i].hit( vPage ) ) {
                    physical_address  = ( TLBE[i].get_pfn() << 8) | offset;
                    if( argc == 2) {
                        cout << "  tlb hit, physical address is      0x" 
                             << setw(4) << setfill('0') << hex << physical_address << endl;
                    }
                    CME[TLBE[i].get_pfn()].set_useVector();
                    hit = true;
                    break;
                }
            }
            // check Page table - NO TLB HIT
            if( !hit ) {
                tlb_miss_count++;
                if( argc == 2 ) cout << "  tlb miss" << endl;
                // if page table is a hit
                if( PTE[vPage].hit() ) {
                    physical_address  = ( PTE[vPage].get_pfn() << 8) | offset;
                    if( argc == 2 ) {
                        cout << "  page hit, physical address is     0x" 
                             << setw(4) << setfill('0') << hex << physical_address << endl;
                    }
                    // update
                    TLBE[FIFO].update( vPage, PTE[vPage].get_pfn() );
                    // checks whether a page hit updates or sets based on use vector
                    unsigned char pre_shift = CME[TLBE[FIFO].get_pfn()].get_useVector() & 0x8f;
                    if(  pre_shift == 0x80 ) CME[TLBE[FIFO].get_pfn()].update( vPage );
                    else CME[TLBE[FIFO].get_pfn()].set_useVector();
                    hit = true;
                }
                // check Core Map table - NO PAGE TABLE HIT
                else {
                    bool full_frame = true;
                    page_fault_count++;
                    if( argc == 2 ) cout << "  page fault" << endl;
                    for( i = 0; i < num_PF; i++ ) {
                        if( CME[i].free() ) {
                            // update
                            CME[i].update( vPage );
                            PTE[vPage].update( i );
                            TLBE[FIFO].update( vPage, PTE[vPage].get_pfn() );
                            physical_address  = ( PTE[vPage].get_pfn() << 8) | offset;
                            if( argc == 2 ) {
                                cout << "  unused page frame allocated" << endl
                                     << "  physical address is               0x" 
                                     << setw(4) << setfill('0') << hex << physical_address << endl;
                            }
                            full_frame = false;
                            break;
                        }
                    }
                    // Core Map Table had no unused page frames
                    if( full_frame ) {
                        if( argc == 2 ) cout << "  page replacement needed" << endl;
                        int lowest = 0; // psuedo-LRU policy
                        for( i = 1; i < num_PF; i++ ) 
                            if( CME[lowest].get_useVector() > CME[i].get_useVector() ) lowest = i;
                        if( argc == 2 ) 
                            cout << "  TLB invalidate of vpn 0x" << CME[lowest].get_vpn() << endl;
                        PTE[CME[lowest].get_vpn()].invalidate();
                        if( argc == 2 ) cout << "  replace frame " << lowest << endl;
                        // update
                        CME[lowest].update( vPage );
                        PTE[vPage].update( lowest );
                        TLBE[FIFO].update( vPage, PTE[vPage].get_pfn() );
                        physical_address = ( PTE[vPage].get_pfn() << 8 | offset );
                        if( argc == 2 ) {
                            cout << "  physical address is               0x" 
                            << setw(4) << setfill('0') << hex << physical_address << endl;
                        }
                    }
                }
                if( argc == 2 ) {
                    cout << "  tlb update of vpn 0x" << setw(4) << setfill('0') << hex << vPage
                         << " with pfn 0x" << setw(2) << setfill('0')  << ( int ) TLBE[FIFO].get_pfn() << endl;
                }
                FIFO++; // increment FIFO order for TLB since it had to go through PTE/CME
            }
            // evenly divisible acccess to 4, or every 4th access, will shift use vector
            if( ( access_count % num_UP ) == 0 ) {
                for( i = 0; i < num_PF; i++ ) CME[i].shift_useVector();
                if( argc == 2 ) cout << "shift use vectors" << endl;
            }
            access_count++;
        }
    }
    print_Statistic( access_count, tlb_miss_count, page_fault_count );
    if( argc == 2) print_Verbose( num_TE, num_PF, TLBE, CME, PTE );
}   

/******************************************** helper functions **********************************************/
void print_Statistic( int access, int tlb, int page )
{
    cout << endl
         << "statistics " << endl
         << "  access      = " << dec << ( access - 1 ) << endl
         << "  tlb misses  = " << dec << tlb << endl
         << "  page faults = " << dec << page << endl;
}

void print_Verbose( int num_TE, int num_PF, TLB_Entry *TLBE, Core_Map_Entry *CME, Page_Table_Entry *PTE )
{
    int i;
    cout << endl << "tlb" << endl;
    for( i = 0; i < num_TE; i++ ) {
        cout << "  valid = " << TLBE[i].get_valid() 
             << ", vpn = 0x" << setw(4) << setfill('0') << hex << ( int ) TLBE[i].get_vpn()
             << ", pfn = 0x" << setw(2) << setfill('0') << ( int ) TLBE[i].get_pfn() 
             << endl;
    }
    cout << endl << "core map table" << endl;
    for( i = 0; i < num_PF; i++ ) {
        cout << "  pfn = 0x" << setw(2) << setfill('0') << i << dec
             << ": valid = " << ( int ) CME[i].get_valid()
             << ", use vector = 0x" << setw(2) << setfill('0') << hex << ( int ) CME[i].get_useVector()
             << ", vpn = 0x" << setw(4) << setfill('0') << CME[i].get_vpn()
             << endl;
    }
    cout << endl << "first ten entries of page table" << endl;
    for( i = 0; i < 10; i++ ) {
        cout << "  vpn = 0x" << setw(4) << setfill('0') << i
             << ": presence = " << ( int ) PTE[i].get_presence()
             << ", pfn = 0x" << setw(2) << setfill('0') << ( int ) PTE[i].get_pfn()
             << endl;
    }
}
