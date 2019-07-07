table Person {
    int id;
    int age;
    string name;
    int job_id;
};

table Job {
    int id;
    string title;
    float pay;
};

new Job {
    0, "CEO", 35.5
};

new Person {
    0, 25, "James Simpson", 0
};

Person.select(age, name);

Person.update(age = age + 1);

Person.select(age, name);

Person.select(Person.name, Job.title, Job.pay).inner_join(Job).on(Person.job_id == Job.id).where(Job.id == 0);