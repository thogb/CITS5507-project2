BEGIN {
    printf("fish_amount,sim_steps,process_num,thread_num,schedule,duration\n");
}

NF == 6 {
    split($1, a, "=");
    split($2, b, "=");
    split($3, c, "=");
    split($4, d, "=");
    split($5, e, "=");
    split($6, f, "=");
    printf("%s,%s,%s,%s,%s,%s\n", a[2], b[2], c[2], d[2], e[2], f[2]);
}