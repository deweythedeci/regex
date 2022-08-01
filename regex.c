#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Regex {
	char* str;
	struct Regex* next;
	struct Regex* star;
	struct Regex* alt;
} Regex;

Regex* genRegex(char* str);
int matchRegex(char* str, Regex* r);
char* substr(char* s, int a, int b);
int stripParen(char* s);
void concatRegex(Regex* r1, Regex* r2);
void starRegex(Regex* r);
void starRegexHelper(Regex* r1, Regex* r2);
void unionRegex(Regex* r1, Regex* r2);
int getUnion(char* str);
int getStar(char* str);
int getGroup(char* str);
void printRegex(Regex* r);

int main(int argc, char** argv){
	if(argc != 3){
		printf("usage: %s string pattern\n", argv[0]);
		return 0;
	}
	
	Regex* r = genRegex(argv[2]);
	printRegex(r);
	int m = matchRegex(argv[1], r);
	printf("match? %d", m);
}

// generates a regular expression from a string
// supports | * and ()
Regex* genRegex(char* str){
	Regex* r = (Regex*) malloc(sizeof(Regex));
	
	if(strlen(str) == 0){
		return NULL;
	}
	
	int p = stripParen(str);
	if(p != 0){
		return genRegex(substr(str, p, strlen(str) - p));
	}
	
	int u = getUnion(str);
	if(u != strlen(str)){
		char* s1 = substr(str, 0, u);
		char* s2 = substr(str, u + 1, strlen(str));
		Regex* r1 = genRegex(s1);
		Regex* r2 = genRegex(s2);
		unionRegex(r1, r2);
		return r1;
	}
	
	int k = getStar(str);
	if(k < 0){
		char* s1 = substr(str, 0, -k);
		char* s2 = substr(str, -k, strlen(str));
		Regex* r1 = genRegex(s1);
		Regex* r2 = genRegex(s2);
		concatRegex(r1, r2);
		return r1;
	}
	else if(k != strlen(str)){
		char* s1 = substr(str, 0, k);
		char* s2 = substr(str, k + 1, strlen(str));
		Regex* r1 = genRegex(s1);
		Regex* r2 = genRegex(s2);
		starRegex(r1);
		concatRegex(r1, r2);
		return r1;
	}
	
	int g = getGroup(str);
	if(g != strlen(str)){
		char* s1 = substr(str, 0, g);
		char* s2 = substr(str, g, strlen(str));
		Regex* r1 = genRegex(s1);
		Regex* r2 = genRegex(s2);
		concatRegex(r1, r2);
		return r1;
	}
	
	r->str = str;
	return r;
}

int matchRegex(char* str, Regex* r){
	if(strlen(str) == 0 && r == NULL){
		return 1;
	}
	for(int i = 0; i < strlen(r->str); i++){
		if(str[i] != r->str[i]){
			if(r->alt != NULL){
				return matchRegex(str, r->alt);
			}
			else{
				return 0;
			}
		}
	}
	return matchRegex(str + strlen(r->str), r->next) || matchRegex(str + strlen(r->str), r->star);
}

// return the substr s[a:b]
char* substr(char* s, int a, int b){
	char* s2 = malloc(b-a+1 * sizeof(char));
	strncpy(s2, s + a, b-a);
}

// gets stripable parens from string (e.g. "((a))" -> 2)
int stripParen(char* s){
	int i = 0;
	while(s[i] == '(' && s[strlen(s) - 1 - i] == ')'){
		i++;
	}
	return i;
}

// concatenate 2 regexes
void concatRegex(Regex* r1, Regex* r2){
	if(r1->alt != NULL){
		concatRegex(r1->alt, r2);
	}
	if(r1->next != NULL){
		concatRegex(r1->next, r2);
	}
	else{
		r1->next = r2;
	}
}

// kleene star a regex
void starRegex(Regex* r){
	starRegexHelper(r, r);
}

void starRegexHelper(Regex* r1, Regex* r2){
	if(r1->alt != NULL){
		starRegexHelper(r1->alt, r2);
	}
	if(r1->next != NULL){
		starRegexHelper(r1->next, r2);
	}
	else{
		r1->star = r2;
	}
}

// union 2 regexes
void unionRegex(Regex* r1, Regex* r2){
	if(r1->alt != NULL){
		unionRegex(r1->alt, r2);
	}
	else{
		r1->alt = r2;
	}
}

int getUnion(char* str){
	int paren = 0;
	int i = 0;
	while(i < strlen(str)){
		if(str[i] == '('){
			paren++;
		}
		else if(str[i] == ')'){
			paren--;
		}
		else if(str[i] == '|' && paren == 0){
			return i;
		}
		else if(str[i] == '\\'){
			i++;
		}
		i++;
	}
	return i;
}

int getStar(char* str){
	if(strlen(str) < 2){
		return strlen(str);
	}
	if(str[1] == '*'){
		return 1;
	}
	int paren = 0;
	int i = 0;
	while(i < strlen(str)){
		if(str[i] == '('){
			paren++;
		}
		else if(str[i] == ')'){
			paren--;
			if(paren == 0 && str[i+1] == '*'){
				return i + 1;
			}
		}
		else if(paren == 0 && str[i] == '*'){
			return 1 - i;
		}
		else if(str[i] == '\\'){
			i++;
		}
		i++;
	}
	return i;
}

int getGroup(char* str){	
	if(str[0] == '('){
		int paren = 1;
		int i = 0;
		while(i < strlen(str)){
			if(str[i] == '('){
				paren++;
			}
			else if(str[i] == ')'){
				paren--;
				if(paren == 0){
					return i + 1;
				}
			}
			else if(str[i] == '\\'){
				i++;
			}
			i++;
		}
	}
	else{
		int i = 0;
		while(i < strlen(str)){
			if(str[i] == '('){
				return i;
			}
			else if(str[i] == '\\'){
				i++;
			}
			i++;
		}
		return strlen(str);
	}	
}

void printRegex(Regex* r){
	if(r == NULL){
		return;
	}
	printf("%s at %p\nnext is %p\nalt is %p\nstar is %p\n\n", r->str, r, r->next, r->alt, r->star);
	printRegex(r->next);
	printRegex(r->alt);
}