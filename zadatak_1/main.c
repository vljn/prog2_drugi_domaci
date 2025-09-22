#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 128

typedef struct pacijent {
    char ime_prezime[31];
    char jmbg[14];
    int hitnost;
    char odeljenje[31];
    int krevet;
} Pacijent;

typedef struct odeljenje {
    char naziv[31];
    int kreveti;
} Odeljenje;

typedef struct cvor_pacijent {
    Pacijent *pacijent;
    struct cvor_pacijent *sledeci;
} CvorPacijent;

typedef struct cvor_odeljenje {
    Odeljenje *odeljenje;
    struct cvor_odeljenje *sledeci;
} CvorOdeljenje;

Pacijent *ucitaj_pacijenta(FILE *f) {
    char linija[MAX_LEN];
    if (!fgets(linija, MAX_LEN, f)) return NULL;
    Pacijent *p = malloc(sizeof(Pacijent));
    if (!p) {
        printf("MEM_GRESKA");
        return NULL;
    }
    char *nl = strchr(linija, '\n');
    if (nl) {
        *nl = '\0';
    }
    sscanf(linija, "%[^,],%[^,],%d,%30[^\n]", p->ime_prezime, p->jmbg, &p->hitnost, p->odeljenje);
    p->krevet = -1;
    return p;
}

CvorPacijent *ucitaj_pacijente(FILE *f) {
    Pacijent *p;
    CvorPacijent *glava = NULL, *pr = NULL;
    while ((p = ucitaj_pacijenta(f))) {
        CvorPacijent *t = malloc(sizeof(CvorPacijent));
        if (!t) {
            printf("MEM_GRESKA");
            return NULL;
        }
        t->sledeci = NULL;
        t->pacijent = p;
        if (!glava) {
            glava = t;
        }
        else {
            pr->sledeci = t;
        }
        pr = t;
    }

    return glava;
}

Odeljenje *ucitaj_odeljenje(FILE *f) {
    char linija[MAX_LEN];
    if (!fgets(linija, MAX_LEN, f)) return NULL;
    Odeljenje *o = malloc(sizeof(Odeljenje));
    if (!o) {
        printf("MEM_GRESKA");
        return NULL;
    }
    char *nl = strchr(linija, '\n');
    if (nl) {
        *nl = '\0';
    }
    sscanf(linija, "%[^,],%d", o->naziv, &o->kreveti);
    return o;
}

CvorOdeljenje *ucitaj_odeljenja(FILE *f) {
    CvorOdeljenje *glava = NULL, *p = NULL;
    Odeljenje *o = NULL;
    while ((o = ucitaj_odeljenje(f))) {
        CvorOdeljenje *t = malloc(sizeof(CvorOdeljenje));
        if (!t) {
            printf("MEM_GRESKA");
            return NULL;
        }
        t->odeljenje = o;
        t->sledeci = NULL;
        if (!glava) {
            glava = t;
        }
        else {
            p->sledeci = t;
        }
        p = t;
    }
    return glava;
}

void ispisi_pacijente(FILE *f, CvorPacijent *pacijenti, int n, int nl) {
    if (nl) fputc('\n', f);
    while (pacijenti && n) {
        Pacijent *p = pacijenti->pacijent;
        fprintf(f, "%s,%s,%d,%s\n", p->ime_prezime, p->jmbg, p->hitnost, p->odeljenje);

        n--;
        pacijenti = pacijenti->sledeci;
    }
}

void oslobodi_pacijente(CvorPacijent *lista) {
    while (lista) {
        CvorPacijent *t = lista;
        lista = lista->sledeci;
        free(t->pacijent);
        free(t);
    }
}

void oslobodi_odeljenja(CvorOdeljenje *lista) {
    while (lista) {
        CvorOdeljenje *t = lista;
        lista = lista->sledeci;
        free(t->odeljenje);
        free(t);
    }
}

int godina_rodjenja(Pacijent *p) {
    char godina[5];
    godina[4] = '\0';
    strncpy(godina + 1, p->jmbg + 4, 3);
    if (godina[1] == '0') godina[0] = '2';
    else godina[0] = '1';
    return atoi(godina);
}

int uporedi(CvorPacijent *c1, CvorPacijent *c2) {
    if (strcmp(c1->pacijent->odeljenje, c2->pacijent->odeljenje) == 0) {
        if (c1->pacijent->hitnost == c2->pacijent->hitnost) {
            return godina_rodjenja(c1->pacijent) - godina_rodjenja(c2->pacijent);
        }
        return c2->pacijent->hitnost - c1->pacijent->hitnost;
    }
    return strcmp(c1->pacijent->odeljenje, c2->pacijent->odeljenje);
}

void sortiraj(CvorPacijent *pacijenti) {
    CvorPacijent *glava = pacijenti;
    CvorPacijent *t = pacijenti;
    while (t) {
        CvorPacijent *sl = t->sledeci;
        while (sl) {
            if (uporedi(t, sl) > 0) {
                Pacijent *temp = t->pacijent;
                t->pacijent = sl->pacijent;
                sl->pacijent = temp;
            }

            sl = sl->sledeci;
        }

        t = t->sledeci;
    }
}

Odeljenje *nadji_odeljenje(CvorOdeljenje *odeljenja, char *naziv) {
    while (odeljenja) {
        if (strcmp(odeljenja->odeljenje->naziv, naziv) == 0) {
            return odeljenja->odeljenje;
        }
        odeljenja = odeljenja->sledeci;
    }
}

void dodeli_krevete(CvorPacijent *pacijenti, CvorOdeljenje *odeljenja) {
    while (pacijenti) {
        Odeljenje *odeljenje = nadji_odeljenje(odeljenja, pacijenti->pacijent->odeljenje);
        pacijenti->pacijent->krevet = odeljenje->kreveti--;

        pacijenti = pacijenti->sledeci;
    }
}

void ispisi_pacijente_kreveti(FILE *f, CvorPacijent *pacijenti) {
    while (pacijenti) {
        Pacijent *p = pacijenti->pacijent;
        if (p->krevet <= 0) {
            pacijenti = pacijenti->sledeci;
            continue;
        }
        fprintf(f,"\n%s,%s,%s,%d", p->ime_prezime, p->jmbg, p->odeljenje, p->krevet);

        pacijenti = pacijenti->sledeci;
    }
}

int main(void) {
    FILE *f_pacijenti = fopen("patients.csv", "r");
    if (!f_pacijenti) {
        printf("DAT_GRESKA");
        return 0;
    }
    FILE *f_bolnica = fopen("hospital.csv", "r");
    if (!f_bolnica) {
        printf("DAT_GRESKA");
        fclose(f_pacijenti);
        return 0;
    }
    FILE *f_izlaz = fopen("department_plan.txt", "w");
    if (!f_izlaz) {
        printf("DAT_GRESKA");
        fclose(f_pacijenti);
        fclose(f_bolnica);
        return 0;
    }
    int n;
    scanf("%d", &n);

    CvorPacijent *pacijenti = ucitaj_pacijente(f_pacijenti);
    if (!pacijenti) {
        fclose(f_pacijenti);
        fclose(f_bolnica);
        fclose(f_izlaz);
        return 0;
    }
    ispisi_pacijente(f_izlaz, pacijenti, n, 0);
    CvorOdeljenje *odeljenja = ucitaj_odeljenja(f_bolnica);
    if (!odeljenja) {
        oslobodi_pacijente(pacijenti);
        fclose(f_pacijenti);
        fclose(f_bolnica);
        fclose(f_izlaz);
        return 0;
    }

    sortiraj(pacijenti);
    ispisi_pacijente(f_izlaz, pacijenti, n, 1);

    dodeli_krevete(pacijenti, odeljenja);
    ispisi_pacijente_kreveti(f_izlaz, pacijenti);


    fclose(f_pacijenti);
    fclose(f_bolnica);
    fclose(f_izlaz);
    oslobodi_pacijente(pacijenti);
    oslobodi_odeljenja(odeljenja);
    return 0;
}