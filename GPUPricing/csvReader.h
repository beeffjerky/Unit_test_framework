#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <utility>
#include <cstdio>
#include <exception>
#include <cassert>
#include <cerrno>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace chrono;

namespace csv_io{

enum ColumnType{
  Int=0, Long, Float, Double, Char, String
};

namespace error{
  const int max_file_name_length = 1024;
  const int max_column_name_length = 63;
  
  struct base : std::exception{
    base(): errno_value(0), line(-1)
    {}

    virtual void errorMessage()const = 0;
    
    const char*what()const throw(){
      errorMessage();
      return error_message_buffer;
    }

    void set_file_name(const char*file_name){
      std::strncpy(this->file_name, file_name, max_file_name_length);
      this->file_name[max_file_name_length] = '\0';
    }

    void set_errno(int evalue) { errno_value = evalue; }
    void set_line(int l) { line = l; }
    void set_column_name(const char*column_name){
      std::strncpy(this->column_name, column_name, max_column_name_length);
      this->column_name[max_column_name_length] = '\0';
    }

    int errno_value;
    int line;
    mutable char error_message_buffer[1024];
    char file_name[max_file_name_length+1];
    char column_name[max_column_name_length+1];
  };

  struct can_not_open_file : base
  {
    void errorMessage()const{
      if(errno_value != 0)
        std::sprintf(error_message_buffer, "Can not open file %s: %s.",
                     file_name, std::strerror(errno_value));
      else
        std::sprintf(error_message_buffer, "Can not open file %s.", file_name);
    }
  };

  struct exceed_line_length_limit : base
  {
    void errorMessage()const{
      std::sprintf(error_message_buffer, 
        "Line #%d in file %s exceeds the max length(2^24-1)", line, file_name);
    }
  };

  const int max_column_content_length = 63;

  struct with_column_content{
    with_column_content(){
      std::memset(column_content, 0, max_column_content_length+1);
    }
    
    void set_column_content(const char*column_content){
      std::strncpy(this->column_content, column_content, max_column_content_length);
      this->column_content[max_column_content_length] = '\0';
    }

    char column_content[max_column_content_length+1];
  };

  struct too_few_columns : base
  {
    void errorMessage()const{
      std::sprintf(error_message_buffer, 
        "file %s has too few columns in line %d.", file_name, line);
    }
  };

  struct escaped_string_not_closed : base
  {
    void errorMessage()const{
      std::sprintf(error_message_buffer,
        "file %s has unclosed escaped string in line %d.", file_name, line);
    }
  };

  struct no_digit : base, with_column_content
  {
    void errorMessage()const{
      std::sprintf(error_message_buffer, 
        "Column %s in line %d contains invalid digit %s.",
        column_name, line, column_content);
    }
  };

  struct invalid_single_char: base
  {
    void errorMessage()const{
      std::sprintf(error_message_buffer, 
        "Column %s in line %d is not a char", column_name, line);
    }
  };
}

template<unsigned _count = 0>
class semaphore{
private:
  mutex mtx;
  condition_variable cv;
  int count;

public:
  semaphore() : count(_count) { }
  void notify() {
    unique_lock<mutex> lck(mtx);
    ++count;
    cv.notify_one();
  }
  void wait() {
    unique_lock<mutex> lck(mtx);
    while (count == 0)
      cv.wait(lck);
    count--;
  }
};

static const int block_len = 1 << 27;
static char buffer[3 * block_len];
static int block_begin[100];
static int block_end[100];
static int data_begin;
static int data_end;
static unsigned file_line[100]; // to report where error occurs

template<unsigned thread_count>
class LineReader{
private:
  FILE* file;

  std::mutex mtx;
  std::condition_variable cv;
  int finished_block_cnt;

  char file_name[error::max_file_name_length+1];

  void open_file(const char*file_name){
    file = std::fopen(file_name, "rb");
    if(file == 0){
      int x = errno;
      error::can_not_open_file err;
      err.set_errno(x);
      err.set_file_name(file_name);
      throw err;
    }
  }

  void init(){
    finished_block_cnt = 0;
    for (unsigned i = 0; i < thread_count; i++)
      file_line[i] = 0;

    // do the buffering ourself.
    std::setvbuf(file, 0, _IONBF, 0);
    /*
    try{
      buffer = new char[3*block_len];
    }catch(...){
      std::fclose(file);
      throw;
    }
    */
    data_begin = 0;
    
    high_resolution_clock::time_point _start = high_resolution_clock::now();
    data_end = (int)std::fread(buffer, 1, 2*block_len, file);
    nanoseconds ns = duration_cast<nanoseconds>(high_resolution_clock::now() - _start);
    cerr << " reading stream@ " << (ns.count() / 1000000) << " ms " << endl;

    // Ignore UTF-8 BOM
    if (data_end >= 3 && buffer[0] == '\xEF' && buffer[1] == '\xBB' && buffer[2] == '\xBF')
      data_begin = 3;
    setBlocks();
  }

  void setBlocks()
  {
    block_begin[0] = data_begin;
    int unit = (std::min)(data_end - data_begin, block_len) / thread_count;
    int lastIdx = thread_count - 1;
    for (unsigned i = 0; i < thread_count; i++) {
      int line_end = (std::max)(0, (std::min)(data_end, 
                                      block_begin[i] + (i==0 ? 1024+unit : unit) - 1));
      while (buffer[line_end] != '\n' && line_end != data_end){
        ++line_end;
      }
      block_end[i] = line_end;
      if (i<lastIdx)
        block_begin[i + 1] = line_end + 1;
    }
  }

public:
  explicit LineReader() : finished_block_cnt(0) {}
  explicit LineReader(const std::string&file_name) : finished_block_cnt(0) {
    open(file_name);
  }

  void open(std::string file_name) {
    set_file_name(file_name.c_str());
    open_file(file_name.c_str());
    init();
  }

  void set_file_name(const char*file_name){
    strncpy(this->file_name, file_name, error::max_file_name_length);
    this->file_name[error::max_file_name_length] = '\0';
  }
  const char*get_file_name()const { return file_name; }
  void set_file_line(unsigned file_line, int threadNo){ this->file_line[threadNo] = file_line; }
  unsigned get_file_line(int threadNo)const { return file_line[threadNo]; }
  int get_block_begin(int threadNo) const{ 
    cerr << endl << "\"";
    for (int i = 0; i < 60; i++)
      cerr << buffer[block_begin[threadNo] -30 + i];
    cerr << "\"" << endl;
    return block_begin[threadNo]; 
  }

  char *next_line(int threadNo){
    if (block_begin[threadNo] >= block_end[threadNo] && data_begin == data_end) {
      std::unique_lock<std::mutex> lck(mtx);
      finished_block_cnt = -1;
      cv.notify_all();
      //cerr << "Thread " << threadNo << " finished." << endl;
      return 0;
    }

    ++file_line[threadNo];
    if (block_end[threadNo] > block_len * 2) {
      cerr << "thread " << threadNo << " : " << block_end[threadNo] << ">" << block_len * 2 << endl;
      assert(block_end[threadNo] <= block_len * 2);
    }
    bool startAfterBlock = false;
    if (block_begin[threadNo] >= block_end[threadNo]) {
      std::unique_lock<std::mutex> lck(mtx);
      //cerr << "Thread " << threadNo << ": " << block_begin[threadNo] << ">="
      //  << block_end[threadNo] << " " << data_begin << "==" << data_end << endl;
      if (finished_block_cnt < 0) {
        //cerr << "Thread " << threadNo << " finished too." << endl;
        return 0;
      } else if (++finished_block_cnt < thread_count) {
        //cerr << "Thread " << threadNo << " blocked." << finished_block_cnt<< endl;
        cv.wait(lck);
        startAfterBlock = true;
        //cerr << "Thread " << threadNo << " released." << endl;
        if (finished_block_cnt < 0) {
          //cerr << "Thread " << threadNo << " finished too." << endl;
          return 0;
        }
      } else {
        int lenRead = (int)std::fread(buffer + data_end, 1, block_len, file);

        //cerr << ": Thread " << threadNo << " feed in more data: " << lenRead << endl;
        if (lenRead <= 0 && block_end[thread_count - 1] == data_end) {
          finished_block_cnt = -1;
          cv.notify_all();
          return 0;
        } 
        data_end += lenRead;
        int start_at = block_end[thread_count - 1] + 1;
        std::memcpy(buffer, buffer + start_at, start_at);
        data_begin = 0;
        data_end -= start_at;
        if (lenRead > 0) {
          std::memcpy(buffer + start_at, buffer + start_at * 2, block_len);
          //cerr << "--";
          //for (int i = 0; i < 90; i++)
          //  cerr << buffer[start_at - 30 + i];
          //cerr << "--" << endl;
        }
        setBlocks();

        finished_block_cnt = 0;
        cv.notify_all();
        startAfterBlock = true;
      }
    }

    int line_end = block_begin[threadNo];
    while (buffer[line_end] != '\n' && line_end != block_end[threadNo]){
      ++line_end;
    }

    if(line_end > data_begin - 1 + block_len){
      cerr << "Thread " << threadNo << " exceed_line_length_limit." << endl;
      error::exceed_line_length_limit err;
      err.set_file_name(file_name);
      err.set_line(file_line[threadNo]);
      throw err;
    }

    if(buffer[line_end] == '\n'){
      buffer[line_end] = '\0';
    }else{
      // in case of the missing newline at the end of the last line
      ++data_end;
      buffer[line_end] = '\0';
    }

    // handle windows \r\n
    if (line_end != block_begin[threadNo] && buffer[line_end - 1] == '\r')
      buffer[line_end-1] = '\0';

    char*ret = buffer + block_begin[threadNo];
    block_begin[threadNo] = line_end + 1;
    data_begin = (std::max)(data_begin, block_begin[threadNo]);
    //if (startAfterBlock)
    //  cerr << "[Thread " << threadNo << "]:::: " << ret << endl;
    return ret;
  }

  ~LineReader(){
    close();
  }

  void close() {
    //delete[] buffer;
    //buffer = NULL;
    if (file) {
      std::fclose(file);
      file = NULL;
    }
  }
};

typedef unsigned ignore_column;
static const ignore_column ignore_no_column = 0;
static const ignore_column ignore_extra_column = 1;
static const ignore_column ignore_missing_column = 2;

static const double bases[] = {
  1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0, 
  1000000000.0, 10000000000.0, 100000000000.0, 1000000000000.0, 10000000000000.0, 
  100000000000000.0, 1000000000000000.0, 10000000000000000.0, 100000000000000000.0, 
  1000000000000000000.0, 10000000000000000000.0, 100000000000000000000.0, 
  1000000000000000000000.0, 10000000000000000000000.0, 100000000000000000000000.0
};

class parser{
public:
  template<char tch>
  static void trim(char*&str_begin, char*&str_end){
    while(*str_begin == tch && str_begin != str_end)
      ++str_begin;
    while(*(str_end-1) == tch && str_begin != str_end)
      --str_end;
    *str_end = '\0';
  }

  template<char sep>
  static const char*find_next_column_end(const char*col_begin){
    while(*col_begin != sep && *col_begin != '\0')
      ++col_begin;
    return col_begin;
  }

  template<char separator>
  static void chop_next_column( char*&line, char*&col_begin, char*&col_end){
    assert(line != nullptr);

    col_begin = line;
    // the col_begin + (... - col_begin) removes the constness
    col_end = col_begin + (find_next_column_end<separator>(col_begin) - col_begin);
    
    if(*col_end == '\0'){
      line = nullptr;
    }else{
      *col_end = '\0';
      line = col_end + 1;	
    }
  }

  template<char separator>
  static void parse_line( char*line, char**sorted_col, const std::vector<int>&col_order){
    for(std::size_t i=0; i<col_order.size(); ++i){
      //if(line == nullptr)
      //  throw error::too_few_columns();
      char*col_begin, *col_end;
      chop_next_column<separator>(line, col_begin, col_end);

      if(col_order[i] != -1){
        trim<' '>(col_begin, col_end);
        sorted_col[col_order[i]] = col_begin;
      }
    }
  }

  static void parse(const char*col, char &x){
    if(!*col)
      throw error::invalid_single_char();
    x = *col;
    ++col;
    if(*col)
      throw error::invalid_single_char();
  }
  
  static void parse(const char*col, char*&x){ strcpy(x, col); }

  template<class T>
  static void parse_uint(const char*col, T&x){
    T tMAX = ((std::numeric_limits<T>::max)() - 9) / 10;
    x = 0;
    while(*col != '\0'){
      if('0' <= *col && *col <= '9'){
        T y = *col - '0';
        if(x > tMAX){
          x = (std::numeric_limits<T>::max)();
          return;
        }
        x = 10*x+y;
      }else
        throw error::no_digit();
      ++col;
    }
  }

  template<class T>
  static void parse_int(const char*col, T&x){
    if(*col == '-'){
      ++col;

      x = 0;
      while(*col != '\0'){
        if('0' <= *col && *col <= '9'){
          T y = *col - '0';
          if(x < ((std::numeric_limits<T>::min)()+y)/10){
          	x = (std::numeric_limits<T>::min)();
          	return;
          }
          x = 10*x-y;
        }else
          throw error::no_digit();
        ++col;
      }
      return;
    }else if(*col == '+')
      ++col;
    parse_uint(col, x);
  }	

  static void parse(const char*col, signed int &x) { parse_int(col, x); }
  static void parse(const char*col, signed long &x) { parse_int(col, x); }
  
  template<class T>
  static void parse_float(const char*col, T&x){
    bool is_neg = false;
    if(*col == '-'){
      is_neg = true;
      ++col;
    }else if(*col == '+')
      ++col;

    x = 0;
    while('0' <= *col && *col <= '9'){
      int y = *col - '0';
      x *= 10;
      x += y;
      ++col;
    }
    
    int sz = sizeof(bases)/sizeof(double);
    int dec_pos = 0; // #digits after the decimal point
    if(*col == '.'|| *col == ','){
      ++col;

      long long x_dec = 0;
      while('0' <= *col && *col <= '9'){
        //pos /= 10;
        int y = *col - '0';
        ++col;
        x_dec *= 10;
        x_dec += y;
        dec_pos ++;
      }
      if (dec_pos<10)
        x = x*(T)bases[dec_pos % sz] + x_dec;
      else {
        //cerr << "$$$$$$$ " << setprecision(20) << x << "+" << x_dec << "/" << bases[dec_pos % sz] << "=" << setprecision(20) << x + (double)x_dec / bases[dec_pos % sz] << endl;
        x += x_dec / (T)bases[dec_pos % sz];
        dec_pos = 0;
      }
    }

    if(*col == 'e' || *col == 'E'){
      ++col;
      int e;

      parse_int(col, e);
      
      if(e != 0){
        e -= dec_pos;
        if(e < 0){
          e = -e;
          x = x / (T)bases[e % sz];
        } else {
          x = x * (T)bases[e % sz];
        }
      }
    } else {
      if (dec_pos > 0)
        x = x / (T)bases[dec_pos % sz];
      if(*col != '\0')
        throw error::no_digit();
    }

    if(is_neg)
      x = -x;
  }

  static void parse(const char*col, float&x) { parse_float(col, x); }
  static void parse(const char*col, double&x) { parse_float(col, x); }
  
  template<class T> void parse(const char*col, T&x){
    static_assert(sizeof(T)!=sizeof(T), 
      "TYPE not supported by parse. Only support int, long, float, double, char, and char*");
  }
};

static char*(row[5000]); // col_count*thread_count

template<unsigned col_count, unsigned thread_count = 1, char separator = ',', char commentor = '#'>
class CSVReader{
private:
  LineReader<thread_count> in;

  std::vector<std::string> column_names;
  std::vector<ColumnType> column_types;
  std::vector<int>col_order;
  size_t memSize;
  unsigned int column_count;

  void set_columns(std::string cols[], ColumnType colTypes[], unsigned int colNum){
    if (colNum > 0) {
      column_count = colNum ;
      column_names.resize(column_count);
      column_types.resize(column_count);
      col_order.resize(column_count);
    }
    memSize = 0;
    for (unsigned int i=0; i<column_count; i++) {
      column_names[i] = cols[i];
      switch (colTypes[i])
      {
      case Int:
        memSize += sizeof(int);
        break;
      case Float:
        memSize += sizeof(float);
        break;
      case Double:
        memSize += sizeof(double);
        break;
      case String:
        memSize += 100; // Default string length
      default:
        memSize += sizeof(char*);
        break;
      }      
      column_types[i] = colTypes[i];
      col_order[i] = i;
    }
  }
  
public:
  explicit CSVReader(): in(), column_count(col_count)
  {
    init();
  }

  explicit CSVReader(std::string fname): in(fname), column_count(col_count)
  {
    init();
  }

  void open(std::string fname) {
    in.open(fname);
  }

  void close() {
    in.close();
  }

  size_t set_header(std::string cols[], ColumnType colTypes[], unsigned int colNum=0){
    set_columns(cols, colTypes, colNum);
    std::fill(row, row + column_count*thread_count, nullptr);
    return memSize;
  }

  bool has_column(const std::string&name) const {
    return col_order.end() != std::find(col_order.begin(), col_order.end(),
        std::find(std::begin(column_names), std::end(column_names), name) 
      - std::begin(column_names));
  }

  void set_file_name(const std::string&file_name)
  { in.set_file_name(file_name.c_str()); }

  void set_file_line(unsigned file_line){ in.set_file_line(file_line); }

  unsigned get_file_line()const{ return in.get_file_line(); }

private:
  void init()
  {
    std::fill(row, row + column_count*thread_count, nullptr);
    col_order.resize(column_count);
    column_names.resize(column_count);
    column_types.resize(column_count);
    for(unsigned i=0; i<column_count; ++i)
      col_order[i] = i;
    for(unsigned i=1; i<=column_count; ++i)
      column_names[i-1] = "col"+std::to_string(i);
  }

  bool parse_helper(void* data, int threadNo){
    //char* data = new char[memSize];
    char* p = (char*)data;
    for (std::size_t r=0; r<column_count; r++) {
      int ri = column_count*threadNo + r;
      const char* one_row = row[ri];
      if(one_row){
        try{
          switch (column_types[r])
          {
          case Int:
            parser::parse(one_row, *((int*)p));
            p += sizeof(int);
            break;
          case Long:
            parser::parse(one_row, *((long*)p));
            p += sizeof(long);
            break;
          case Float:
            parser::parse(one_row, *((float*)p));
            p += sizeof(float);
            break;
          case Double:
            parser::parse(one_row, *((double*)p));
            p += sizeof(double);
            break;
          case Char:
            parser::parse(one_row, *p);
            p += sizeof(char);
            break;
          case String:
            parser::parse(one_row, p);
            p += strlen(p);
            *p='\0';
            ++p;
            break;
          default:
            parser::parse(one_row, p);
            p += sizeof(char*);
            break;
          }
        }catch(error::with_column_content&err){
          err.set_column_content(row[ri]);
          throw;
        }
      }
    }
    return true;
  }

  static bool is_comment(const char*line)
  { return *line == commentor; }

public:
  std::string bypass_row() {
    try{
      char*line;
      do{
        line = in.next_line(0);
        if(!line)
          return "";
      }while(is_comment(line));
      return line;
    }catch(error::can_not_open_file& err){
      err.set_file_name(in.get_file_name());
      throw;
    }
  }

  bool read_row(void* data, int threadNo){
    if (column_types.size()>column_count)
      throw("columns less than specified");
    if (column_types.size()<column_count)
      throw("columns more than specified");
    try{
      char*line;
      do{
        try {
          line = in.next_line(threadNo);
        }
        catch (...) {
          cerr << "Thread " << threadNo << " got trouble next_line." << endl;
        }
        if (line==nullptr || strlen(line) == 0)
          return false;
      } while (is_comment(line));
      try{
        parser::parse_line<separator>(line, &row[column_count*threadNo], col_order);
      }
      catch (error::too_few_columns &err){
        err.set_file_name(in.get_file_name());
        err.set_line(in.get_file_line(threadNo));
        cerr << err.what();
        cerr << " $$$ Thread " << threadNo << " got trouble @"
          << in.get_block_begin(threadNo) << ": " << line << endl;
        for (int i = 0; i < 60; i++)
          cerr << line[i];
      }
      catch (...) {
        cerr << " $$$ Thread " << threadNo << " got trouble @" 
          << in.get_block_begin(threadNo) << ": " << line << endl;
        for (int i = 0; i < 60; i++)
          cerr << line[i];
      }
      return parse_helper(data, threadNo);
    }
    catch (error::can_not_open_file&err){
      err.set_file_name(in.get_file_name());
      err.set_line(in.get_file_line(threadNo));
      cerr << err.what();
      throw err;
    }
  }
};

}
