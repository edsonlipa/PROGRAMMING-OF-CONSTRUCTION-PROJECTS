#include <bits/stdc++.h>
using namespace std;

#define n_periodos 10 // numero de periodos
#define n_i 10000 // numero de generaciones
#define n_p 50 // tamanho de la poblacion
#define n_a 51 //numero de actividades
#define n_g 51 //numero de genes
#define n_e 10 //nuemero de estructuras -> para este caso serian casas
#define m_p 0.02 // probabilidad de mutacion
#define mo_d 2622834.77 // mano de obra por dia por actividad
#define costo_casa  98080122.0 // costo de estructura (casa) $ 98.080.122
#define porc_incial 30 // porcentaje del costo de la casa
std::vector<double> costos_en_material={9668412.15,1933682.5,6905009.15,1657442,6353528.41};
std::vector<int> casas_vendidas={2,2,1,2,2,1};// casas vendidas por periodo , entiendace el primer vendido son el el periodo 0 ,antes de la contruccion
#define ind_x_torneo 2
class periodo {
  public:
    int id;
    double ingresos;
    double egresos;
    double flujo_de_caja;
    std::vector<int> iniciados;
    std::vector<int> finalizados;
    periodo (){
      ingresos=0;
      egresos=0;
      flujo_de_caja=0;
    };
    virtual ~periodo (){

    };
};

class actividad {
  public:
    int codigo; //el codigo
    string descripcion;
    int durac_max;
    int durac_min;
    int retraso_maximo;
    int predecesor;//ids de actividades predecesoras
    bool act_terminal;
    actividad (int c,string des,int max, int min,int ret_m):codigo(c),descripcion(des),durac_max(max),durac_min(min),retraso_maximo(ret_m){
      predecesor=-1;
      act_terminal=false;

    };
    virtual ~actividad (){};
};

class Individuo {
  public:
    std::vector<int> duracion;
    std::vector<int> retraso;
    std::vector<int> dias_iniciales;
    std::vector<int> dias_finales;
    double flujo_de_caja;
    int duracion_total;
    std::vector<periodo> programacion;//timeline del periodo
    Individuo (){
      duracion=std::vector<int> (n_a);
      retraso=std::vector<int> (n_a);
      dias_iniciales=std::vector<int> (n_a);
      dias_finales=std::vector<int> (n_a);
      programacion= vector<periodo>(n_periodos,periodo());
      duracion_total=0;
      flujo_de_caja=0;
    };
    virtual ~Individuo (){};
};

std::vector<actividad> actividades;
std::vector<Individuo> poblacion;
Individuo el_mejor;
double obtener_flujo_de_caja(std::vector<periodo> &programa) {
  double min=99999999999;
  for (size_t i = 0; i < programa.size(); i++) {
    programa[i].flujo_de_caja=programa[i].ingresos-programa[i].egresos;
    if (programa[i].flujo_de_caja<min) {
      min=programa[i].flujo_de_caja;
    }
  }
  return min;
}
void reiniciar_egresos_ingresos(Individuo &indiv) {
    for (int i = 0; i < n_periodos; i++) {
      indiv.programacion[i].egresos=0;
    }
    double presupuesto=casas_vendidas[0]*((costo_casa*porc_incial)/100);
    for (size_t j = 0; j < n_periodos; j++) {
      if (j==0) {
        indiv.programacion[j].ingresos=presupuesto+casas_vendidas[j+1]*(costo_casa*porc_incial)/100;
      }else if(j < casas_vendidas.size()-1){
        indiv.programacion[j].ingresos=casas_vendidas[j+1]*(costo_casa*porc_incial)/100;
      }else{
        indiv.programacion[j].ingresos=0;
      }
    }
}

void actualizar_individuo(Individuo &indiv) {
  reiniciar_egresos_ingresos(indiv);
  // std::cout << "nuevo indi" << '\n';
  int tiempo_inicial=indiv.retraso[0];
  indiv.dias_iniciales[0]=tiempo_inicial;
  tiempo_inicial+=indiv.duracion[0];
  indiv.dias_finales[0]=tiempo_inicial;
  int tiempo;
  for (int a = 1; a < n_a; a++) {
    // std::cout << "actividad "<<a <<" nombre "<<actividades[a].descripcion<< '\n';
    if (actividades[a].predecesor<0) {
      tiempo=tiempo_inicial;
    }
    int dia_inicio=tiempo+indiv.retraso[a];
    int dia_fin=dia_inicio+indiv.duracion[a];
    // std::cout << "dia inicial "<<dia_inicio<<" dia fin"<<dia_fin << '\n';
    indiv.dias_iniciales[a]=dia_inicio;
    indiv.dias_finales[a]=dia_fin;
    tiempo=dia_fin;
    int p_i=dia_inicio/30;
    int p_f=dia_fin/30;
    // std::cout << "periodo "<<p_i<<" periodo"<<p_f << '\n';
    // for (size_t i = 0; i < 10; i++) {
    //   std::cout << "egresos" <<indiv.programacion[i].egresos << '\n';
    //   /* code */
    // }
    if (p_i!=p_f) {
      for (int i = p_i; i < p_f; i++) {
        int tope=(i+1)*30;
        // std::cout << "dia inicio "<<dia_inicio<<" dia tope "<<tope << '\n';
        int dias_restantes=abs(tope-dia_inicio);
        int porcentaje=(dias_restantes*100)/30;
        double mo=dias_restantes*mo_d;
        double m=(costos_en_material[(actividades[a].codigo-2)%5]*porcentaje)/100;

        indiv.programacion[i].egresos+=mo+m;
        dia_inicio=tope;
      }
      int base=(p_f)*30;
      int dias_restantes=abs(dia_fin-base);
      int porcentaje=(dias_restantes*100)/30;
      double mo=dias_restantes*mo_d;
      double m=(costos_en_material[(actividades[a].codigo-2)%5]*porcentaje)/100;
      indiv.programacion[p_f].egresos+=mo+m;
    }
    if (actividades[a].act_terminal) {
      // std::cout << "ACTIVIDAD TEMRINADA "<<actividades[a].descripcion  << '\n';
      int period=dia_fin/30;
      indiv.programacion[period].ingresos+=costo_casa*(0.7);
      // indiv.programacion[period].ingresos+=1;
    }
    // std::cout  << '\n';

  }
  indiv.flujo_de_caja= obtener_flujo_de_caja(indiv.programacion); //fitness
  // std::cout  << '\n';
}

void load_table(string inputFileName) {

    ifstream inputFile(inputFileName);
    int l = 0;

    while (inputFile) {
        l++;
        string s;
        if (!getline(inputFile, s)) break;
        if (s[0] != '#') {
            istringstream ss(s);
            std::vector<int> temp;
            string desc;
            for (size_t i = 0; i < 5; i++) {
              string line;
              getline(ss, line, ',');
              try {
                  temp.push_back(stof(line));
              }
              catch (const std::invalid_argument e) {
                  desc=line;
                  e.what();
              }
            }

            if(l>1){
              actividad n_act(temp[0],desc,temp[1],temp[2],temp[3]);
              if ((temp[0]-2)%5!=0&&temp[0]!=1) {
                n_act.predecesor=temp[0]-1;
              }
              if ((temp[0]-1)%5==0) {
                n_act.act_terminal=true;
              }
              actividades.push_back(n_act);
            }
        }
    }

}

void Inicializar_poblacion(){
  el_mejor.flujo_de_caja=-9999999999999;

  for (size_t i = 0; i < n_p; i++) {
    Individuo nuevo;
    for (size_t j = 0; j < n_a; j++) {
        nuevo.duracion[j]=(rand()%(actividades[j].durac_max-actividades[j].durac_min+1)+actividades[j].durac_min);
        nuevo.retraso[j]=rand()%(actividades[j].retraso_maximo+1);
    }
    double presupuesto=casas_vendidas[0]*((costo_casa*porc_incial)/100);
    for (size_t j = 0; j < casas_vendidas.size()-1; j++) {
      if (j==0) {
        nuevo.programacion[j].ingresos=presupuesto+casas_vendidas[j+1]*(costo_casa*porc_incial)/100;
      }else{
        nuevo.programacion[j].ingresos=casas_vendidas[j+1]*(costo_casa*porc_incial)/100;
      }
    }
    poblacion.push_back(nuevo);
  }
}
void estado_de_actividades() {
  for (size_t i = 0; i < n_a; i++) {
    if (actividades[i].predecesor<0) {
      std::cout << "actividad "<<actividades[i].codigo <<" inicial, sin predecesores" << '\n';
    }
    if (actividades[i].act_terminal) {
      std::cout << "actividad "<<actividades[i].codigo <<" terminal, indica la temrinacion de una casa o edificio" << '\n';
    }
  }
}
void mostrar_periodos(std::vector<periodo> programa) {
  for (size_t i = 0; i < programa.size(); i++) {
    std::cout << "periodo " <<i<< '\n';
    std::cout << "\tingresos " <<programa[i].ingresos<< '\n';
    std::cout << "\tegresos " <<programa[i].egresos<< '\n';
    std::cout << "\tflujo de caja " <<programa[i].flujo_de_caja<< '\n';
  }
}

void mostrar_programa(Individuo indiv) {
    for (size_t i = 0; i < indiv.programacion.size(); i++) {
      std::cout << "periodo " <<i<< '\n';
      std::cout << "\tingresos " <<indiv.programacion[i].ingresos<< '\n';
      std::cout << "\tegresos " <<indiv.programacion[i].egresos<< '\n';
      std::cout << "\tflujo de caja " <<indiv.programacion[i].flujo_de_caja<< '\n';
    }
}

void mostrar_poblacion() {
  for (size_t i = 0; i < n_p; i++) {
    std::cout << "\n[ ";
    for (size_t j = 0; j < n_a; j++) {
        std::cout << poblacion[i].duracion[j]<<"," ;
    }
    std::cout << "]\n[ ";
    for (size_t j = 0; j < n_a; j++) {
      std::cout << poblacion[i].retraso[j]<<"," ;
    }
    std::cout << "]\n";
  }
}
void mostrar_fitness_de_poblacion() {
  for (size_t i = 0; i < poblacion.size(); i++) {
    std::cout << i<<"  "<<poblacion[i].flujo_de_caja<< '\n';
  }std::cout  << '\n';
}
void mostrar_el_mejor() {
  std::cout <<"la mejor solucion encontrada  "<<el_mejor.flujo_de_caja<< '\n';
  mostrar_periodos(el_mejor.programacion);
}
class Compare
{
public:
    bool operator() (pair<int,unsigned> F1, pair<int,unsigned>F2)
    {
        return F1.second>F2.second;
    }
};
bool repeat_ind(int rand_t[],int index){
  for (size_t i = 0; i < index; i++) {
    if (rand_t[i]==rand_t[index]) {
      return true;
    }
  }
  return false;
}
int tournament() {
  // std::cout << "seleccion por torneo" << '\n';
  int rand_t[ind_x_torneo];
  bool repeat=true;
  priority_queue<pair<unsigned,double>,std::vector<pair<unsigned,double>>,Compare> pq;
  for (size_t i = 0; i < ind_x_torneo; i++){
        rand_t[i]=rand()%n_p;
        while (repeat_ind(rand_t,i)) {
          rand_t[i]=rand()%n_p;
        }
    pq.push(make_pair(rand_t[i],poblacion[rand_t[i]].flujo_de_caja));
  }
  return pq.top().first;
}

void cruzamiento(int ind1,int ind2) {
  Individuo hijo1=poblacion[ind1];
  Individuo hijo2=poblacion[ind2];

  int mid=n_g/2;
  for (size_t i = 0; i <= mid; i++) {//se intercambia los 3 ultimos bits

    swap(hijo1.retraso[i],hijo2.retraso[i]);
    swap(hijo1.duracion[i],hijo2.duracion[i]);
  }
  actualizar_individuo(hijo1);
  actualizar_individuo(hijo2);

  poblacion.push_back(hijo1);
  poblacion.push_back(hijo2);
}
void seleccion_y_cruzamiento(){
  int indiv1=tournament();
  int indiv2=tournament();while (indiv1==indiv2)indiv2=tournament();
  cruzamiento(indiv1,indiv2);
}
void mutacion(Individuo &indv) {
  int gen=rand()%n_g;
  indv.duracion[gen]=(rand()%(actividades[gen].durac_max-actividades[gen].durac_min+1)+actividades[gen].durac_min);
  indv.retraso[gen]=rand()%(actividades[gen].retraso_maximo+1);
}
bool compare_fitnes(Individuo a, Individuo b)
{
  return a.flujo_de_caja>b.flujo_de_caja;
}
void seleccionar_siguiente_poblacion() {
  sort(poblacion.begin(),poblacion.end(),compare_fitnes);
  while (poblacion.size()>n_p) {
    poblacion.pop_back();
    }
  if (poblacion.begin()->flujo_de_caja>el_mejor.flujo_de_caja) {
    el_mejor=*poblacion.begin();
  }
}
int main(int argc, char const *argv[]) {
   cout.precision(9);
   load_table("../actividades.csv");
   srand(time(NULL));
   Inicializar_poblacion();
   // mostrar_poblacion();

   for (size_t t = 0; t < n_i; t++) {
    // std::cout << "\tGeneracion "<<t+1 << '\n';
     for (size_t i = 0; i < n_p; i++) {
       actualizar_individuo(poblacion[i]);
     }
     // mostrar_periodos(poblacion[t].programacion);
     // mostrar_fitness_de_poblacion();

     seleccion_y_cruzamiento();
     for (size_t i = 0; i < n_p; i++) {
       float mutar=(float)(rand()%100)/100;
       if (mutar<m_p) {
         mutacion(poblacion[i]);
       }
     }
     seleccionar_siguiente_poblacion();
     // mostrar_fitness_de_poblacion();
   }
   std::cout << "\tcosto_casa " <<costo_casa*10<< '\n';
   mostrar_el_mejor();
  return 0;
}
