#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <mpi.h>

namespace chrono = std::chrono;

class Graph {
  std::vector<std::vector<int>> _adjLists;

public:
  Graph(int V);
  void addEdge(int origem, int destino);
  std::vector<int> find_triangles();
};

Graph::Graph(int V) { _adjLists.resize(V + 1); }

void Graph::addEdge(int origem, int destino) {
  _adjLists[origem].push_back(destino);
  _adjLists[destino].push_back(origem);
}

std::vector<int> Graph::find_triangles() {
  int N = _adjLists.size() + 1;
  int rank;


  std::vector<int> answer(N);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Datatype tLinha, tColuna;

  // Cria um tipo para as colunas
  MPI_Type_vector(N, 1, N, MPI_DOUBLE, &tColuna);
  MPI_Type_commit(&tColuna);

  // Cria um tipo para as linhas
  MPI_Type_vector(N, 1, N, MPI_INT, &tLinha);
  MPI_Type_commit(&tLinha);


  for (unsigned i = 0; i < _adjLists.size(); i++) { // edge
    int triangles{0};

    if (_adjLists[i].size() >  1){ // Dessa forma não percorre vetores que não podem possuem triangles
      for (unsigned j = 0; j < _adjLists[i].size(); j++) { // vertex
        int buffer{_adjLists[i][j]};
        for (unsigned h = j + 1; h < _adjLists[i].size(); h++) {       // Desta forma eu consigo pegar todas as combinacoes possiveis para os numeros
          int buffer2{_adjLists[i][h]};
          for (unsigned k = 0; k < _adjLists[buffer].size(); k++) {
            if (buffer2 == _adjLists[buffer][k]) {
              triangles++;
              break;
            }
          }
        }
      }
    }
    answer[i] = triangles; // TODO: VER SE ELE ESTA RESPONDENDO DE                                      //FORMA CORRETA
  }
  return answer;
}

std::string read_argument(int argc, char *argv[]) {
  std::string filename;

  // We need exactly two arguments (considering program name).
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <filename>\n";
    exit(1);
  }

  filename = argv[1];
  return filename;
}

std::pair<int, std::vector<int>> read_numbers(std::string filename) {
  std::pair<int, std::vector<int>> data;

  std::vector<int> numbers;
  int the_highest_number{0};

  std::ifstream infile;
  infile.open(filename);

  if (infile.is_open()) {
    int num;
    while (infile >> num) {
      if (the_highest_number < num) the_highest_number = num;
      numbers.push_back(num);
    }
  }
  data.first = the_highest_number;
  data.second = numbers;

  infile.close();
  return data;
}

int main(int argc, char *argv[]) {

  std::string filename = read_argument(argc, argv);

  int buffer{-1};
  int contador{0};
  int contador1{0};
  int inserir;

  double elapsed = 0;
  chrono::high_resolution_clock::time_point t1, t2;

  std::vector<int> numbers;
  int the_highest_number;
  std::pair<int, std::vector<int>> data;

  std::vector<int> answer;




  data = read_numbers(filename); // Obtendo tanto os valores referentes a tarefa
                              // quanto o maior numero do grapho para a alocacao

  // Separando os dados que forma obtidos pelo read_numbers
  the_highest_number = std::get<0>(data);
  numbers = std::get<1>(data);

  // The time monitor
  t1 = chrono::high_resolution_clock::now();

  Graph g(the_highest_number); // Inicializando o grapho

  // Colocando os dados nele
  for (unsigned i = 0; i < numbers.size(); i++) {
    inserir = numbers[i];
    if (contador == 1) {
      g.addEdge(buffer, inserir);
      contador = 0;
    } else {
      contador += 1;
      contador1 += 1;
    }
    buffer = inserir;
  }

  answer = g.find_triangles();

  t2 = chrono::high_resolution_clock::now();

  auto dt = chrono::duration_cast<chrono::microseconds>(t2 - t1);
  elapsed += dt.count();

  // Show timing results
  std::cout << filename << " Time Taken: " << elapsed / 1.0 / 1e6 << std::endl;

  // Criadno um arquivo com .trg
  std::string toReplace(".edgelist");
  size_t pos = filename.find(toReplace);
  filename.replace(pos, toReplace.length(), ".TO_REMOVE");
  std::ofstream output_file;
  output_file.open(filename);

  // Colocando no output_file
  for (std::vector<int>::iterator it = answer.begin(); it != answer.end() - 1; ++it)
   {output_file << *it << " ";}

  output_file.close();


  MPI_Finalize();

  return 0;
}
