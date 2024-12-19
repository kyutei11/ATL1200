/*  ATL1200.c : Copyright (c) 2001 Koichi TAKASAKI. All rights reserved. */

#include<stdio.h>

#define COM 1
#define INC 2
#define DEC 3
#define RJMP 4
#define RCALL 5
#define RET 6
#define SBI 7
#define CBI 8
#define SBIC 9
#define SBIS 10
#define SBRC 11
#define SBRS 12
#define IN 13
#define OUT 14
#define MOV 15
#define ADD 16
#define SUB 17
#define AND 18
#define OR 19
#define CP 20
#define BREQ1 21
#define BRNE1 22
#define BRSH1 23
#define BRLO1 24
#define LDI 25
#define ANDI 26
#define ORI 27
#define CPI 28
#define LSR 29

#define REG 1
#define IO  0
#define NUM 2

FILE *fp,*fp2; /* pointer of the code */
char o, code[2048];
int i,j,count,pline,cp,cps,srno,lplev,cont,iflag;

/* writer */

void pioset(char o) {}
void pioput(char o) {}
char pioget(void) {}

/* writer */
void sck()
{pioput(o|1);for(j=0;j<5;j++){};pioput(o&0xfe);}
void mosi(unsigned char hl)
{
    if(hl) o=o|4; else o=o&0xfb;
    pioput(o);
}
unsigned char miso(void)
{return((pioget()>>1)&1);}

unsigned char readc(void)
{
    unsigned char c=0;
    for(i=7;i>=0;i--){c+=(miso()<<i);sck();}
    return(c);
}
void writec(unsigned char c)
{for(i=7;i>=0;i--){mosi((c>>i)&1);sck();}}

void beginprog(void)
{
    pioput(10);pioput(0);
    writec(0xac); writec(0x53);
    writec(0x00); writec(0x00);
}
 void cleardev()
{writec(0xac);writec(0x80);writec(0);writec(0);}

void writeprog(int addr,unsigned int dat)
{
    writec(0x48); writec(addr>>8); writec(addr&0x00ff); writec(dat>>8);
    writec(0x40); writec(addr>>8); writec(addr&0x00ff); writec(dat&0x00ff);
    for(i=0;i<100;i++) ;
}
unsigned int readprog(int addr)
{
    unsigned int dat=0;
    writec(0x28); writec(addr>>8); writec(addr&0x00ff); dat=readc()<<8;
    writec(0x20); writec(addr>>8); writec(addr&0x00ff); dat+=readc();
    return(dat);
}
/* editor */

int cp0(int d,int cp)
{
    if(d)
        if(code[cp+1]!=0)
            do cp++; while(code[cp]!=0x0a);
    if((d==0)&&(cp>0))
        while(code[cp]!=0x0a) cp--; 
    return(cp);
}

void list1(char d)
{
    int cpm,cpp;
    cpm=cp0(0,cp);cpp=cp0(1,cp);
    if((d==0)&&(cpm>0))
    {
        cpp=cp0(0,cpm-1);cpm=cpp;
        if(cpp>0)
            for(i=0;i<=4;i++)
                if((cpm=cp0(0,cpm-1))==0) break;
        for(i=i;i<4;i++) printf("\n");
    }
    for(cp=cpm+1;cp<=cpp;cp++)
        printf("%c",code[cp]);
    cp=cpp;
}

void ledit(char k)
{
    char wid,s[20];
    int cpm;
    i=0;
    if(k!='I') /*delete single line*/
    {
        i=cp0(0,cp);cp=cp0(0,i-1);wid=i-cp;
        while(code[i++]!=0) code[i-wid]=code[i]; 
    }
    if(k!='D') /*edit and insert single line*/
    {
        printf(":");scanf("%s",s);
        for(i=0;i<20;i++) if(s[i]==',') s[i]=' ';
        wid=strlen(s)+1;
        i=strlen(code);
        cpm=cp0(0,cp);
        if(i+wid>=2046)
            printf("exceeded code area.\n");
        else
        {
            for(j=i;j>cpm;j--) code[j+wid]=code[j];
            for(j=1;j<wid;j++) code[cpm+j]=s[j-1];
            code[cpm+wid]=0x0a;
        }
    }
    cp=cp0(1,cp);
}

/* compiler */

void alert(char *m)
{
    printf("\nIn line %d: %s\n",pline,m);
    cont=0; cps=cp;
}

void opcode(char comm, unsigned char r, unsigned char d, int jmp, char k)
{
    unsigned int act,dum;
    
    switch(comm)
    {
        case COM:
            act = (d<<4) + 0x9400; break;
        case INC:
            act = (d<<4) + 0x9403; break;
        case DEC:
            act = (d<<4) + 0x940a; break;
        case LSR:
            act = (d<<4) + 0x9406; break;
        case RJMP:
            act=0xc000 + (jmp&0x0fff); break;
        case RCALL:
            act=0xd000 + (jmp&0x0fff); break;
        case RET:
            if(iflag) act= 0x9518;
            else act = 0x9508;
            break;
        case SBI:
            act = 0x9a00 + ((0x1f&r)<<3) + (0x07&d); break;
        case CBI:
            act = 0x9800 + ((0x1f&r)<<3) + (0x07&d); break;
        case SBIC:
            act = 0x9900 + ((0x1f&r)<<3) + (0x07&d); break;
        case SBIS:
            act = 0x9b00 + ((0x1f&r)<<3) + (0x07&d); break;
        case SBRC:
            act = 0x1f&r;
            act = 0xfc00 + (act<<4) + (0x07&d);
        break;
        case SBRS:
            act = 0x1f&r;
            act = 0xfe00 + (act<<4) + (0x07&d);
        break;
        case IN:
            dum = (r&0x30)<<5;
            act = (d&0x1f)<<4;
            act += 0xb000 + dum+(0x0f&r); 
        break;
        case OUT:
            dum = (d&0x30)<<5;
            act = (r&0x1f)<<4;
            act += 0xb800 + dum+(0x0f&d); 
        break;
        case MOV: case ADD: case SUB: case AND: case OR: case CP:
            dum = (0x1f&d)<<4;
            act = r;
            act = (act&0x0010)<<5;
            act += dum +(0x0f&r);
            switch(comm)
            {
                case MOV  : act += 0x2c00 ; break;
                case ADD  : act += 0x0c00 ; break;
                case SUB  : act += 0x1800 ; break;
                case AND  : act += 0x2000 ; break;
                case OR   : act += 0x2800 ; break;
                case CP   : act += 0x1400 ; break;
            }
        break;
        
        case BREQ1: act = 0xf009; break;
        case BRNE1: act = 0xf409; break;
        case BRSH1: act = 0xf408; break;

        case LDI: case ANDI: case ORI: case CPI:
            dum = (r&0xf0)<<4;
            act = (d&0x0f)<<4;
            act += dum + (r&0x0f);
            switch(comm)
            {
                case LDI:  act += 0xe000; break;
                case ORI:  act += 0x6000; break;
                case ANDI: act += 0x7000; break;
                case CPI:  act += 0x3000; break;
            }
        break;
        default:
            if(comm==BRLO1) act = 0xf008;
    }
    /*if(count>3)*/ printf("%3d %4Xh %2Xh %2Xh %3d\n",count,act,r,d,jmp);

    if(k=='W')
    {
        writeprog(count,act);
        if(readprog(count)!=act)
            printf("verify error:PC=%d\n",count);
    }
    if(++count==512) alert("Max.obj.size exceeded");
}

void readtoend()
{
    while(code[++cp]!=0x0a) ;
    pline++;
}

char fetch()
{
    char c;
    while((c=code[++cp])==0x20) ;
    return(c);
}

unsigned char nget()
{
    char c;
    unsigned char v,val;
    
    val = 0;
    switch(c=fetch())
    {
        case'B':
            for(i=7;i>=0;i--)
                switch(c=fetch())
                {
                    case'0': break;
                    case'1':
                        v = 1; 
                        for(j=0;j<i;j++)
                            v=v<<1;
                        val+=v;
                        break;
                    default: alert("b.const error.");
                }
        break;
        case'H':
            for(i=1;i>=0;i--)
            {
                c=fetch();
                if(c>='A' && c<='F')
                    v = c-'A'+10;
                else if(c>='0' && c<='9')
                    v = c-'0';
                else alert("h.const error.");
                if(i==1) v*=16;
                val+=v;
            }
        break;
        default:
            printf("c=%c",c);
            alert("no B/H");
    }
    return(val);    
}

int vget(char c,unsigned char *v,char reg)
{
    int vtype;
    if((c>='A')&&(c<='Z'))
    {
        vtype = REG;
        *v = c-'A';
    }
    else if(c=='[')
    {
        vtype = IO;
        *v = nget();
    }
    else if(c=='.')
    {
        if(reg==-1) alert("can't use number");
        else if(reg<16) alert("use Q-Z.");
        vtype = NUM;
        *v = nget();
    }
    else
        alert("NO REG I/O NUM.");
    return(vtype);
}

int bget(char *b)
{
    char c;
    int set;
    *b=((c=fetch()) -'0');
    c=fetch();
    if(c=='=')
    {
        c=fetch();
        if(c=='1') set = 1;
        else if(c=='0') set = 0;
        else alert("only 0|1 is allowed.");
    }
    else alert("no (=).");
    return(set);
}

int compile(char k)
{
    int i,j,vtype,srlv,ic,
    lplav[10], /* do-while loop level and pointer */
    srpt[10]; /* PC at subroutine */
    unsigned char v1,v2,vd;
    char b,c,c2,csave,srname[13],srlav[20][13];

    if(k=='W')
    {
        beginprog();cleardev();for(i=0;i<150;i++) ;
        beginprog();
    }
    count = 4; srno = 0; lplev = 0; pline=1; cp=0; cont=1;
    iflag=0; ic=0; 

    while(((c=fetch())!=0)&&(cont))
    {
//        if(k=='C') {while(!kbhit()) ; j=getch();}
        srlv = 0;
        switch(c)
        {
            case 0x0a: pline++; break; /* do nothing */
            case'*':
                if(srno > 0) {opcode(RET,0,0,0,k);iflag=0;} 
                if(srno == 20) alert("too many subroutine.");
                if(code[cp+1]=='*') {cp++; iflag=1; ic=count-1;}
                srpt[srno] = count;
                while((c=code[++cp])!=0x0a) /* until pline end */
                {
                    srlav[srno][srlv] = c;
                    if(srlv++ == 12) alert("subr. name too long."); 
                }
                srlav[srno++][srlv] = 0;
                pline++;
            break;
            
            case'#':
                while((c=code[++cp])!=0x0a) /* until pline end */
                {
                    srname[srlv] = c;
                    if(srlv++ == 12) alert("subr. name too long."); 
                }
                srname[srlv] = 0;
                j=0;
                for(i=0;i<srno;i++) /* search subroutine name */
                    if(strcmp(srlav[i],srname)==0)
                    {
                        j=1;
                        if(i == srno-1) /* call itself */
                            opcode(RJMP,0,0,srpt[i]-count-1,k);
                        else /* call previous routine */
                            opcode(RCALL,0,0,srpt[i]-count-1,k);
                    }
                if(j==0) alert("subroutine not exist");
                pline++;
            break;

            case'(':
                lplav[lplev] = count;
                if(lplev++ == 10) alert("nest too deep.");
                readtoend();
            break;

            case')': case';':
                csave = c;
                vtype = vget(fetch(),&v1,-1);
                if((c=fetch())=='.')
                    if(bget(&b))
                        if(vtype==REG) opcode(SBRC,v1,b,0,k);
                        else opcode(SBIC,v1,b,0,k);
                    else
                        if(vtype==REG)  opcode(SBRS,v1,b,0,k);
                        else opcode(SBIS,v1,b,0,k);
                else if(vtype==REG)
                    switch(c)
                    { /* LHS is destination */
                        case'=': case'~':
                            if(vtype=vget(fetch(),&v2,v1))
                            {
                                if(vtype==REG) opcode(CP,v2,v1,0,k);
                                else opcode(CPI,v2,v1,0,k);
                                if(c=='=') opcode(BRNE1,0,0,0,k);
                                else if(c=='~') opcode(BREQ1,0,0,0,k);
                                else alert("=|~ must be used.");
                            }
                            else alert("RHS is not REG/NUM.");
                        break;

                        case'<': case'>':
                            if((c2=fetch())=='=')
                                if(vget(fetch(),&v2,-1))
                                {
                                    if(c=='<'){vd=v1;v1=v2;v2=vd;}
                                    opcode(CP,v2,v1,0,k);
                                    opcode(BRLO1,0,0,0,k);
                                }
                            else if(vget(c2,&v2,-1))
                            {
                                if(c=='>') {vd=v1;v1=v2;v2=vd;}
                                opcode(CP,v2,v1,0,k);
                                opcode(BRSH1,0,0,0,k);
                            }
                            else alert("RHS is not REG/NUM");
                        break;
                        default: alert("error at branch.");
                    }
                else alert("LHS is not IO.b/REG.");

                if(csave==')')
                {
                    opcode(RJMP,0,0,lplav[--lplev]-count-1,k);
                    readtoend();
                }
            break;
            
            default: /* A..Z,[  */
                if(vget(c,&v1,-1))
                {
                    if((c=fetch())=='=')
                        if((vtype=vget(fetch(),&v2,v1))==REG)
                            opcode(MOV,v2,v1,0,k);
                        else if(vtype==IO)
                            opcode(IN,v2,v1,0,k);
                        else /*NUM*/
                            opcode(LDI,v2,v1,0,k);
                    else
                        switch(c)
                        {
                            case'<':
                                opcode(ADD,v1,v1,0,k);
                            break;
                            case'>':
                                opcode(LSR,0,v1,0,k);
                            break;
                            case'!':
                                opcode(COM,0,v1,0,k);
                            break;
                            case'+':
                                if((c=fetch())=='+')
                                    opcode(INC,0,v1,0,k);
                                else if(c=='=')
                                    if(vget(fetch(),&v2,-1))
                                        opcode(ADD,v2,v1,0,k);
                                    else alert("RHS must be A-Z.");
                                else alert("subst. error.");
                            break;
                            case'-':
                                if((c=fetch())=='-')
                                    opcode(DEC,0,v1,0,k);
                                else if(c=='=')
                                    if(vget(fetch(),&v2,-1))
                                        opcode(SUB,v2,v1,0,k);
                                    else alert ("RHS must be A-Z.");
                                else alert("subst. error.");
                            break;
                            case'|':
                                if((vtype=vget(fetch(),&v2,v1))==REG)
                                    opcode(OR,v2,v1,0,k);
                                else if(vtype==NUM)
                                    opcode(ORI,v2,v1,0,k);
                                else alert ("RHS is not REG/NUM.");
                            break;
                            case'&':
                                if((vtype=vget(fetch(),&v2,v1))==REG)
                                    opcode(AND,v2,v1,0,k);
                                else if(vtype==NUM)
                                    opcode(ANDI,v2,v1,0,k);
                                else alert ("RHS is not REG/NUM.");
                            break;
                            default:
                                alert("subst. error.");
                        }
                }
                else /* c=='[':I/O */
                {
                    if((c=fetch())=='.')/* bit subst. is allowed only to I/O*/
                        if(bget(&b))
                            opcode(SBI,v1,b,0,k);
                        else
                            opcode(CBI,v1,b,0,k);
                    else if(c=='=')
                        if(vget(fetch(),&v2,-1))
                            opcode(OUT,v2,v1,0,k);
                        else alert("RHS must be A-Z.");
                }
                readtoend();
            break;
        }
    }
    if(cont)
    {
        i=srpt[srno-1]-1; /* main routine */
        if(ic==0) ic=i;
        opcode(RJMP,0,0,i-count,k);
        
        /* compile end; write vector */
        count = 0;
        opcode(RJMP,0,0,i,k);
        opcode(RJMP,0,0,ic-1,k);
        opcode(RJMP,0,0,ic-2,k);
        opcode(RJMP,0,0,ic-3,k);
    }
    else cp=cps;
    
    printf("finished.\n");
    if(k=='W') pioput(8);
}
 
void files(char *com)
{
    char c,fn[13];
    i=0;
    printf("file name:");
    scanf("%s",fn);
    if((fp = fopen(fn,com)) == NULL)
        printf("not found.\n");
    else if(strcmp(com,"r")==0)
    {
        while((c=fgetc(fp))!=EOF)
            code[i++]=c;
        code[i]=0;/*EOS*/
        cp=1;
    }
    else
        while(code[i])
            fprintf(fp,"%c",code[i++]);
    fclose(fp);
}

int main(void)
{
    char k;
    fp2 = fopen("pio","r+");
    pioset(0xf2);o=0;
    for(cp=0;cp<2048;cp++) code[cp]=0;
    code[0]=0x0a;code[(cp=1)]=0x0a;
    while(1)
//    if(kbhit())
        switch(k=getchar())
        {
            case'L': files("r");break;
            case'S':
                if(code[2]!=0) files("w");
                else printf("no source code.\n");
                break;
            case'8': list1(0); break;
            case'2': list1(1); break;
            case'E': case'D': case'I': ledit(k); break;
            case'C': case'W': compile(k); break;
        }
    fclose(fp2);
}