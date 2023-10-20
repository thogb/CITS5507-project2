BEGIN {
    printf("thread_num,schedule,duration\n");
}

NF == 3 {
    split($1, a, "=");
    split($2, b, "=");
    split($3, c, "=");
    printf("%s,%s,%s\n", a[2], b[2], c[2]);
}