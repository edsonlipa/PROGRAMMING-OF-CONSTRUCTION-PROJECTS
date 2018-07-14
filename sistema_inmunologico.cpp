#include <bits/stdc++.h>
using namespace std;
#define interations 10000
#define n_p 25 // tamanho de la poblacion
#define s_size 25//selection size
#define rand_cells_num 15
#define mutation_factor -1.2
#define clone_rate 0.3
int n_clones = int(n_p * clone_rate);

#define n_periodos 10 // numero de periodos
#define n_a 51 //numero de actividades
#define n_g 51 //numero de genes
#define n_e 10 //nuemero de estructuras -> para este caso serian casas
// #define m_p 0.02 // probabilidad de mutacion
#define mo_d 2622834.77 // mano de obra por dia por actividad
#define costo_casa  98080122.0 // costo de estructura (casa) $ 98.080.122
#define porc_incial 30 // porcentaje del costo de la casa
std::vector<double> costos_en_material={9668412.15,1933682.5,6905009.15,1657442,6353528.41};
std::vector<int> casas_vendidas={2,2,1,2,2,1};// casas vendidas por periodo , entiendace el primer vendido son el el periodo 0 ,antes de la contruccion

class periodo {
  public:
    int id;
    double ingresos;
    double egresos;
    double flujo_de_caja;
    double flujo_de_caja_acumulado;
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
    int durac_max_temp;
    int durac_min_temp;
    int retraso_maximo;
    int retraso_maximo_temp;
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
    double a;//afinidad
    std::vector<periodo> programacion;//timeline del periodo
    Individuo (){
      duracion=std::vector<int> (n_a);
      retraso=std::vector<int> (n_a);
      dias_iniciales=std::vector<int> (n_a);
      dias_finales=std::vector<int> (n_a);
      programacion= vector<periodo>(n_periodos,periodo());
      duracion_total=0;
      flujo_de_caja=0;
      a=0.5;
    };
    virtual ~Individuo (){};
};

std::vector<actividad> actividades;
std::vector<Individuo> poblacion;
Individuo el_mejor;
double obtener_min_flujo_de_caja_acumulado(std::vector<periodo> &programa) {
  double min=99999999999;
  for (size_t i = 0; i < programa.size(); i++) {
    programa[i].flujo_de_caja=programa[i].ingresos-programa[i].egresos;
    if (i>0) {
      programa[i].flujo_de_caja_acumulado=programa[i].flujo_de_caja+programa[i-1].flujo_de_caja_acumulado;
    }else{
      programa[i].flujo_de_caja_acumulado=programa[i].flujo_de_caja;
    }
    if (programa[i].flujo_de_caja_acumulado<min) {
      min=programa[i].flujo_de_caja_acumulado;
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
  indiv.flujo_de_caja= obtener_min_flujo_de_caja_acumulado(indiv.programacion); //fitness
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
    std::cout << "\tingresos \t" <<programa[i].ingresos<< '\n';
    std::cout << "\tegresos \t" <<programa[i].egresos<< '\n';
    std::cout << "\tflujo de caja \t" <<programa[i].flujo_de_caja<< '\n';
    std::cout << "\tflujo_de_caja_acumulado \t" <<programa[i].flujo_de_caja_acumulado<< '\n';
  }
}
void mostrar_programa(Individuo indiv) {
    for (size_t i = 0; i < indiv.programacion.size(); i++) {
      std::cout << "periodo " <<i<< '\n';
      std::cout << "\tingresos \t" <<indiv.programacion[i].ingresos<< '\n';
      std::cout << "\tegresos \t" <<indiv.programacion[i].egresos<< '\n';
      std::cout << "\tflujo de caja \t" <<indiv.programacion[i].flujo_de_caja<< '\n';
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
void mostrar_retorno_de_inversion(double inversion,double utilidad) {
  std::cout << "retorno de inversion :" <<(utilidad-abs(inversion))/abs(inversion) << '\n';
}
void mostrar_el_mejor() {
  std::cout << "\tcosto_casa " <<costo_casa*10<< '\n';
  std::cout << "\tinversion estimada " <<(costo_casa*10)*0.6<< '\n';
  std::cout <<"\tla minima inversion encontrada  "<<el_mejor.flujo_de_caja<< '\n';
  mostrar_retorno_de_inversion(el_mejor.flujo_de_caja,costo_casa*10);
  mostrar_periodos(el_mejor.programacion);
}
void actualizar_maximos_minimos_temporales() {
  for (size_t i = 0; i < n_a; i++) {
    actividades[i].durac_max_temp=actividades[i].durac_min;
    actividades[i].durac_min_temp=actividades[i].durac_max;
    actividades[i].retraso_maximo_temp=0;
    for (size_t j = 0; j < n_p; j++) {
      if (poblacion[j].retraso[i]>actividades[i].retraso_maximo_temp) {
        actividades[i].retraso_maximo_temp=poblacion[j].retraso[i];
      }
      if (poblacion[j].duracion[i]<actividades[i].durac_min_temp) {
        actividades[i].durac_min_temp=poblacion[j].duracion[i];
      }
      if (poblacion[j].duracion[i]>actividades[i].durac_max_temp) {
        actividades[i].durac_max_temp=poblacion[j].duracion[i];
      }
    }
  }
}
void update_afinitys(){
    ///vector<pair<double,vector<bool>>>
  double max_f = -9999999999;
  double min_f = +9999999999;
	for(int i=0 ;i<poblacion.size();i++){
        if(max_f>poblacion[i].flujo_de_caja){
            max_f=poblacion[i].flujo_de_caja;
        }
        if(min_f<poblacion[i].flujo_de_caja){
            min_f=poblacion[i].flujo_de_caja;
        }

  }

	for(int i=0 ;i<poblacion.size();i++){
		if (max_f - min_f == 0){
			poblacion[i].a=1;
        }
		else{
			poblacion[i].a=1-(abs(poblacion[i].flujo_de_caja)/abs( max_f - min_f ));
		}
	}
}
vector<Individuo> clon_hipermutacion(){
	vector<Individuo> tmp;
	for(int i=0;i<n_p/5;i++){
		for(int j=0;j<n_clones;j++){
            Individuo clon;
			clon =poblacion[i];
			double taza_mutacion;
			try{
				 taza_mutacion=exp(mutation_factor * clon.a);
			}
			catch( double ){
				taza_mutacion = 0;
			}
      // std::cout << "tasa de mutation_factor "<<taza_mutacion  << '\n';
			for(int k=0;k<n_g;k++){
				double rand_ = static_cast <double> (rand()) /static_cast <double> (RAND_MAX);
				if( rand_ < taza_mutacion ){
          clon.duracion[k]=(rand()%(actividades[k].durac_max_temp-actividades[k].durac_min+1)+actividades[k].durac_min);
          clon.retraso[k]=rand()%(actividades[k].retraso_maximo_temp+1);
				}
      }
			tmp.push_back(clon);
		}
	}
    return tmp;
}
vector<Individuo> init_random_indiv(){
	vector<Individuo> tmp;
  for (size_t i = 0; i < rand_cells_num; i++) {
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
    tmp.push_back(nuevo);
  }

	return tmp;
}
bool compare_fitnes(Individuo a, Individuo b)
{
  return a.flujo_de_caja>b.flujo_de_caja;
}
void seleccionar_siguiente_poblacion(int iteracion) {
  sort(poblacion.begin(),poblacion.end(),compare_fitnes);
  while (poblacion.size()>n_p) {
    poblacion.pop_back();
    }
  if (poblacion.begin()->flujo_de_caja>el_mejor.flujo_de_caja) {
    el_mejor=*poblacion.begin();
    ofstream myfile;
    myfile.open ("si_plt",std::ofstream::out | std::ifstream::app);
    myfile << iteracion<<"\t"<<el_mejor.flujo_de_caja << '\n';
    myfile.close();
    // std::cout << "se actualizo el mejor: "<<el_mejor.flujo_de_caja <<"en la iteracion "<<iteracion<< '\n';
  }
}

void CLONALG() {
  load_table("../actividades.csv");

  Inicializar_poblacion();
  // mostrar_poblacion();
	for(int t=0;t<interations;t++){
		// cout<<"^^^^^^^^^^^^^^^^^^ iteracion "<<t+1<<"^^^^^^^^^^^^^^^^^^^^^"<<endl;
	//
    for (size_t i = 0; i < n_p; i++) {
        actualizar_individuo(poblacion[i]);
     }
    update_afinitys();
    actualizar_maximos_minimos_temporales();

    vector<Individuo> clones = clon_hipermutacion();
    for (size_t i = 0; i < clones.size(); i++) {
        actualizar_individuo(clones[i]);
        poblacion.push_back(clones[i]);

    }
    vector<Individuo>indivs_random = init_random_indiv();
    for (size_t i = 0; i < indivs_random.size(); i++) {
         actualizar_individuo(indivs_random[i]);
         poblacion.push_back(indivs_random[i]);
     }

  		seleccionar_siguiente_poblacion(t);
	}
}
int main(int argc, char const *argv[]) {
  cout.precision(9);
  ofstream myfile;
  myfile.open ("si_plt",std::ofstream::out | std::ifstream::trunc);
  myfile.clear();
  myfile.close();
  srand(time(0));
  CLONALG();
  // myfile << interations<<"\t"<<el_mejor.flujo_de_caja << '\n';
  mostrar_el_mejor();
  return 0;
}
