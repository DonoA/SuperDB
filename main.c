#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

#include "database.h"
#include "query_lang.h"

/*
 * table Person {
 *   int id;
 *   int age;
 *   string name;
 *   int job_id;
 * };
 */

/*
 * table Job {
 *   int id;
 *   string title;
 *   float pay;
 * };
 */

/*
 * new Job {
 *    0, "CEO", 35.5
 * };
 */

/*
 * new Person {
 *    0, 25, "James Simpson", 0
 * };
 */

/*
 * Person.update(age = age + 1);
 */

/*
 * Person.select(age, name).where(id == 0);
 */

/*
 * (Person, Job).select(Person.name, Job.title, Job.pay).inner_join(Person.job_id == Job.id);
 */

int main(int argc, char *argv[])
{
    database_t * db = create_database("testDB");

    FILE * sql_file = fopen("/home/dallen/projects/SuperDB/test.sql", "r");

    char buf[1024] = { 0 };

    fread(buf, sizeof(char), 1024, sql_file);

    execute_code(db, buf);

    print_table(get_table_id(db, 0));
    print_table(get_table_id(db, 1));
    printf("Done!\n");
    return 0;
}