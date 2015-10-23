/**\file
 *   Written by Ching-ling Huang to test multiple inserts
 */
#include <sys_os.h>
#include <sqlite3.h>
#include <time.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  return 0;
}

int main(int argc, char **argv){
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  int loop_max=10000;
  int count=1;
  time_t start_time, end_time;
  double diff;


  if( argc!=2 ){
    fprintf(stderr, "loop test Usage: %s DATABASE \n", argv[0]);
    exit(1);
  }
//open connection to db
  rc = sqlite3_open(argv[1], &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

//output current time before loop
  start_time = time(NULL);


  rc = sqlite3_exec(db,"insert into tbl1 values(10,10)", callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

while(count<=loop_max)
{
  //update test
  rc = sqlite3_exec(db,"update tbl1 set one=5 where two=10", callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  rc = sqlite3_exec(db,"update tbl1 set one=15 where two=10", callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

count++;
}

//output current time after loop
  end_time = time(NULL);

  diff = difftime (end_time, start_time);
  printf ("It took %.2lf seconds for %d loops\n", diff, loop_max);

  rc = sqlite3_exec(db,"delete tbl1 where two=10", callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

//close connection
  sqlite3_close(db);
  return 0;
}
