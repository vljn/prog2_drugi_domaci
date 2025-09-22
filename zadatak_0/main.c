#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct kupovina {
    char naziv_prodavnice[31];
    double iznos;
    char kategorija[31];
} Kupovina;

typedef struct cvor {
    Kupovina *kupovina;
    struct cvor *sledeci;
} Cvor;

typedef struct cvor_grupa {
    char kategorija[31];
    double ukupno;
    double prosek;
    int broj_kupovina;
    struct cvor_grupa *sledeci;
} CvorGrupa;

void oslobodi_listu(Cvor *lista) {
    while (lista) {
        free(lista->kupovina);
        Cvor *t = lista->sledeci;
        free(lista);
        lista = t;
    }
}

void oslobodi_grupe(CvorGrupa *grupe) {
    while (grupe) {
        CvorGrupa *t = grupe->sledeci;
        free(grupe);
        grupe = t;
    }
}

Cvor *ucitaj(FILE *f) {
    char np[31], k[31];
    double i;
    Cvor *lista = NULL, *tr = NULL;
    while (fscanf(f, "%30[^,],%lf,%30[^\n]\n", np, &i, k) == 3) {
        Cvor *t = malloc(sizeof(Cvor));
        t->sledeci = NULL;
        if (!t) {
            if (lista) oslobodi_listu(lista);
            printf("MEM_GRESKA");
            return NULL;
        }
        Kupovina *kupovina = malloc(sizeof(Kupovina));
        if (!kupovina) {
            free(t);
            if (lista) oslobodi_listu(lista);
            printf("MEM_GRESKA");
            return NULL;
        }
        strcpy(kupovina->naziv_prodavnice, np);
        strcpy(kupovina->kategorija, k);
        kupovina->iznos = i;
        t->kupovina = kupovina;
        if (!lista) {
            lista = t;
        }
        else {
            tr->sledeci = t;
        }
        tr = t;
    }

    return lista;
}

void ispisi(FILE *f, Cvor *lista, int n) {
    while (lista && n) {
        Kupovina *k = lista->kupovina;
        fprintf(f, "%s,%.2lf,%s\n", k->naziv_prodavnice, k->iznos, k->kategorija);
        n--;
        lista = lista->sledeci;
    }
}

int broj_elemenata(Cvor *lista) {
    int br = 0;
    while (lista) {
        br++;
        lista = lista->sledeci;
    }
    return br;
}

CvorGrupa *dodaj(CvorGrupa **lista, char *kategorija) {
    CvorGrupa *novi = malloc(sizeof(CvorGrupa));
    if (!novi) {
        printf("MEM_GRESKA\n");
        return NULL;
    }
    strcpy(novi->kategorija, kategorija);
    novi->ukupno = 0.0;
    novi->prosek = 0.0;
    novi->broj_kupovina = 1;
    novi->sledeci = NULL;
    if (!*lista) {
        *lista = novi;
        return novi;
    }
    CvorGrupa *t = *lista;
    while (t->sledeci) {
        t = t->sledeci;
    }
    t->sledeci = novi;
    return novi;
}

CvorGrupa *nadji_grupu(CvorGrupa *lista, char *kategorija) {
    while (lista) {
        if (strcmp(kategorija, lista->kategorija) == 0) {
            return lista;
        }
        lista = lista->sledeci;
    }
    return NULL;
}

CvorGrupa *grupisi(Cvor *lista) {
    CvorGrupa *grupe = NULL;
    while (lista) {
        CvorGrupa *grupa = nadji_grupu(grupe, lista->kupovina->kategorija);
        if (grupa) {
            grupa->ukupno += lista->kupovina->iznos;
            grupa->broj_kupovina++;
        }
        else {
            CvorGrupa *g = dodaj(&grupe, lista->kupovina->kategorija);
            if (!g) {
                printf("MEM_GRESKA");
                oslobodi_grupe(grupe);
                return NULL;
            }
            g->ukupno += lista->kupovina->iznos;
        }
        lista = lista->sledeci;
    }
    CvorGrupa *t = grupe;
    while (t) {
        t->prosek = t->ukupno / t->broj_kupovina;
        t = t->sledeci;
    }

    return grupe;
}

void ispisi_grupe(FILE *f, CvorGrupa *grupe, int nl) {
    fputc('\n', f);
    while (grupe) {
        fprintf(f ,"%s,%.2lf,%.2lf", grupe->kategorija, grupe->ukupno, grupe->prosek);
        if (grupe->sledeci || nl) fputc('\n', f);

        grupe = grupe->sledeci;
    }
}

int uporedi(CvorGrupa *c1, CvorGrupa *c2) {
    if (c1->ukupno == c2->ukupno) {
        return strcmp(c1->kategorija, c2->kategorija);
    }
    return c1->ukupno - c2->ukupno;
}

void zameni(CvorGrupa *c1, CvorGrupa *c2) {
    int bk = c1->broj_kupovina;
    double pr = c1->prosek, uk = c1->ukupno;
    char k[31];
    strcpy(k, c1->kategorija);
    c1->broj_kupovina = c2->broj_kupovina;
    c1->ukupno = c2->ukupno;
    c1->prosek = c2->prosek;
    strcpy(c1->kategorija, c2->kategorija);
    c2->broj_kupovina = bk;
    c2->ukupno = uk;
    c2->prosek = pr;
    strcpy(c2->kategorija, k);
}

CvorGrupa *sortiraj(CvorGrupa *lista) {
    CvorGrupa *tr = lista;
    while (tr) {
        CvorGrupa *sl = tr->sledeci;
        while (sl) {
            if (uporedi(tr, sl) < 0) {
                zameni(tr, sl);
            }
            sl = sl->sledeci;
        }

        tr = tr->sledeci;
    }
}

CvorGrupa *filtriraj(CvorGrupa *lista, double min) {
    CvorGrupa *t = lista;
    CvorGrupa *p = NULL;
    while (t) {
        if (t->ukupno < min) {
            if (!p) {
                lista = lista->sledeci;
                free(t);
                t = lista;
                continue;
            }
            t = t->sledeci;
            free(p->sledeci);
            p->sledeci = t;
            continue;
        }
        p = t;
        t = t->sledeci;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("ARG_GRESKA");
        return 0;
    }
    const char *FAJL_ULAZ = "spending_log.csv", *FAJL_IZLAZ = "spending_summary.txt";
    int n;
    double min = atof(argv[1]);
    scanf("%d", &n);

    FILE *f_ulaz = fopen(FAJL_ULAZ, "r");
    if (!f_ulaz) {
        printf("DAT_GRESKA");
        return 0;
    }
    FILE *f_izlaz = fopen(FAJL_IZLAZ, "w");
    if (!f_izlaz) {
        fclose(f_ulaz);
        printf("DAT_GRESKA");
        return 0;
    }

    Cvor *lista = ucitaj(f_ulaz);
    if (!lista) {
        fclose(f_ulaz);
        fclose(f_izlaz);
        return 0;
    }
    ispisi(f_izlaz, lista, n);

    CvorGrupa *grupe = grupisi(lista);
    if (!grupe) {
        oslobodi_listu(lista);
        fclose(f_ulaz);
        fclose(f_izlaz);
        return 0;
    }
    ispisi_grupe(f_izlaz, grupe, 1);

    sortiraj(grupe);
    filtriraj(grupe, min);
    ispisi_grupe(f_izlaz, grupe, 0);

    fclose(f_ulaz);
    fclose(f_izlaz);
    oslobodi_listu(lista);
    oslobodi_grupe(grupe);
    return 0;
}