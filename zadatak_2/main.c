#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct karta {
    char naziv[101], tip[21], atribut[11];
    int napad, odbrana, nivo;
} Karta;

typedef struct cvor_karta {
    Karta *karta;
    struct cvor_karta *sledeci;
} CvorKarta;

typedef struct tip {
    char tip[21];
    int broj_karata, ukupno_napad, ukupno_odbrana;
} Tip;

typedef struct cvor_tip {
    Tip *tip;
    struct cvor_tip *sledeci;
} CvorTip;

void oslobodi_karte(CvorKarta *lista) {
    while (lista) {
        CvorKarta *t = lista->sledeci;
        free(lista->karta);
        free(lista);
        lista = t;
    }
}

void oslobodi_tipove(CvorTip *lista) {
    while (lista) {
        CvorTip *t = lista->sledeci;
        free(lista->tip);
        free(lista);
        lista = t;
    }
}

Karta *ucitaj_kartu(FILE *f) {
    char linija[256];
    if (!fgets(linija, 256, f)) return NULL;
    linija[strcspn(linija, "\n")] = '\0';
    Karta *k = malloc(sizeof(Karta));
    if (!k) {
        printf("MEM_GRESKA");
        return NULL;
    }
    sscanf(linija, "%[^,],%d,%d,%d,%[^,],%[^,]", k->naziv, &k->napad, &k->odbrana, &k->nivo,k->tip, k->atribut);
    return k;
}

void filtriraj(CvorKarta **lista, char *atribut) {
    CvorKarta *t = *lista;
    CvorKarta *p = NULL;
    while (t) {
        if (strcmp(atribut, t->karta->atribut) != 0) {
            if (p == NULL) {
                *lista = t->sledeci;
                free(t->karta);
                free(t);
                t = *lista;
            }
            else {
                p->sledeci = t->sledeci;
                free(t->karta);
                free(t);
                t = p->sledeci;
            }
        }
        else {
            p = t;
            t = t->sledeci;
        }
    }
}

CvorKarta *ucitaj_karte(FILE *f) {
    CvorKarta *glava = NULL, *prosli = NULL;
    Karta *k;
    while ((k = ucitaj_kartu(f))) {
        CvorKarta *t = malloc(sizeof(CvorKarta));
        if (!t) {
            printf("MEM_GRESKA");
            if (glava) {
                free(k);
                oslobodi_karte(glava);
            }
            return NULL;
        }
        t->sledeci = NULL;
        t->karta = k;
        if (!glava) {
            glava = t;
        }
        else {
            prosli->sledeci = t;
        }
        prosli = t;
    }
    return glava;
}

void ispisi_karte(FILE *f, CvorKarta *lista, int n) {
    for (int i = 0; lista && (n < 0 || i < n); i++, lista = lista->sledeci) {
        Karta *k = lista->karta;
        fprintf(f, "%s %d %d %d %s %s\n", k->naziv, k->napad, k->odbrana, k->nivo, k->tip, k->atribut);
    }
    fputc('\n', f);
}

CvorTip *nadji_cvor(CvorTip *lista, char *tip) {
    while (lista) {
        if (strcmp(lista->tip->tip, tip) == 0) {
            return lista;
        }

        lista = lista->sledeci;
    }
    return NULL;
}

CvorTip *grupisi(CvorKarta *karte) {
    CvorTip *glava = NULL, *p = NULL;
    while (karte) {
        CvorTip *ct = nadji_cvor(glava, karte->karta->tip);
        if (ct) {
            ct->tip->broj_karata++;
            ct->tip->ukupno_napad += karte->karta->napad;
            ct->tip->ukupno_odbrana += karte->karta->odbrana;
        }
        else {
            CvorTip *n = malloc(sizeof(CvorTip));
            if (!n) {
                printf("MEM_GRESKA");
                // oslobodi
                return NULL;
            }
            n->sledeci = NULL;
            n->tip = malloc(sizeof(Tip));
            n->tip->ukupno_napad = karte->karta->napad;
            n->tip->ukupno_odbrana = karte->karta->odbrana;
            n->tip->broj_karata = 1;
            strcpy(n->tip->tip, karte->karta->tip);
            if (!glava) {
                glava = n;
            }
            else {
                p->sledeci = n;
            }
            p = n;
        }


        karte = karte->sledeci;
    }

    return glava;
}

void ispisi_tipove(FILE *f, CvorTip *tipovi) {
    while (tipovi) {
        Tip *tip = tipovi->tip;
        double prosek_napad = (double) tip->ukupno_napad / tip->broj_karata;
        double prosek_odbrana = (double) tip->ukupno_odbrana / tip->broj_karata;
        fprintf(f,"%-15s %2d %7.2lf %7.2lf", tip->tip, tip->broj_karata, prosek_napad, prosek_odbrana);
        if (tipovi->sledeci) {
            fputc('\n', f);
        }
        tipovi = tipovi->sledeci;
    }
}

int uporedi(CvorTip *c1, CvorTip *c2) {
    if (c1->tip->broj_karata == c2->tip->broj_karata) {
        return strcmp(c1->tip->tip, c2->tip->tip);
    }
    return c2->tip->broj_karata - c1->tip->broj_karata;
}

void sortiraj(CvorTip *glava) {
    while (glava) {
        CvorTip* sl = glava->sledeci;
        while (sl) {
            if (uporedi(glava, sl) > 0) {
                Tip *t = glava->tip;
                glava->tip = sl->tip;
                sl->tip = t;
            }
            sl = sl->sledeci;
        }

        glava = glava->sledeci;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("ARG_GRESKA");
        return 0;
    }
    char *atribut = argv[1];
    int n;
    scanf("%d", &n);
    FILE *f_ulaz = fopen("cards.csv", "r");
    if (!f_ulaz) {
        printf("DAT_GRESKA");
        return 0;
    }
    FILE *f_izlaz = fopen("yu_gi_oh.txt", "w");
    if (!f_izlaz) {
        fclose(f_ulaz);
        printf("DAT_GRESKA");
        return 0;
    }

    CvorKarta *karte = ucitaj_karte(f_ulaz);
    ispisi_karte(f_izlaz, karte, n);

    filtriraj(&karte, atribut);
    ispisi_karte(f_izlaz, karte, -1);

    CvorTip *tipovi = grupisi(karte);
    sortiraj(tipovi);
    ispisi_tipove(f_izlaz, tipovi);


    fclose(f_ulaz);
    fclose(f_izlaz);
    oslobodi_karte(karte);
    oslobodi_tipove(tipovi);
    return 0;
}