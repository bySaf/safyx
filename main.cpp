#include <iostream>
#include <fstream>
#include <cmath>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <set>

const long double EPS = 1e-10;
const int LenAlphabet = 26;
const long double INF = 1e9;

#include <utility> // для std::pair

// Шаблонная функция для создания пары
template <typename T1, typename T2>
std::pair<T1, T2> make_pair_custom(T1 first, T2 second) {
  return std::pair<T1, T2>(first, second);
}

template<typename T>
class List;

template<typename T>
class Node_List {
  friend class List<T>;
  T data;
  Node_List<T> *next = nullptr;
  Node_List<T> *prev = nullptr;

  template<typename T1>
  friend List<T1> Merge(List<T1> first, List<T1> second);

  template<typename T2>
  friend List<T2> MergeSort(List<T2> now);

 public:
  Node_List(T data, Node_List *prev, Node_List *next) : data(data), prev(prev), next(next) {
  };

  Node_List() {
  };
};

template<typename T>
class List {
  Node_List<T> *begin = nullptr;
  Node_List<T> *end = nullptr;
  int size = 0;

  template<typename T1>
  friend List<T1> Merge(List<T1> first, List<T1> second);

  template<typename T2>
  friend List<T2> MergeSort(List<T2> now);

 public:
  List() = default;

  ~List();

  List(const List<T> &now) {
    Clear();
    int ind = 0;
    Node_List<T> *cur = now.begin;

    while (ind < now.size) {
      PushBack(cur->data);
      ind++;
      cur = cur->next;
    }
  }

  void Clear();

  void PushBack(T x);

  void Erase(int pos);

  T &operator[](int ind) const;

  List<T> &operator =(List<T> now) {
    std::swap(begin, now.begin);
    std::swap(end, now.end);
    std::swap(size, now.size);
    return *this;
  }

  void Print(std::ofstream &out, bool flag) const;

  int GetSize() const;

  T &back() const;
};

template<typename T>
void List<T>::PushBack(T val) {
  size++;
  if (begin == nullptr) {
    begin = new Node_List<T>(val, nullptr, nullptr);
    end = begin;
  } else {
    Node_List<T> *now = new Node_List<T>(val, nullptr, nullptr);
    end->next = now;
    now->prev = end;
    end = now;
  }
}

template<typename T>
T &List<T>::operator[](int ind) const {
  Node_List<T> *cur = begin;
  int my_index = 0;
  while (my_index < ind) {
    cur = cur->next;
    my_index++;
  }
  return cur->data;
}

template<typename T>
void List<T>::Erase(int pos) {
  if (pos >= size) {
    return;
  }
  int my_index = 0;
  Node_List<T> *now = begin;
  while (my_index < pos) {
    now = now->next;
    my_index++;
  }
  if (now->prev == nullptr and now->next == nullptr) {
    begin = nullptr;
    end = nullptr;
    delete now;
  } else if (now->prev == nullptr) {
    begin = now->next;
  } else if (now->next == nullptr) {
    end = now->prev;
  } else {
    now->prev->next = now->next;
    now->next->prev = now->prev;
    delete now;
  }
  size--;
}

template<typename T>
void List<T>::Print(std::ofstream &out, bool flag) const {
  int my_index = 0;
  Node_List<T> *element = begin;

  while (my_index < size) {
    if (flag) {
      std::cout << my_index << ") " << element->data.GetString() << std::endl;
    } else {
      out << element->data.GetString() << std::endl;
    }
    element = element->next;
    my_index++;
  }
}

template<typename T>
List<T>::~List() {
  Clear();
}

template<typename T>
void List<T>::Clear() {
  Node_List<T> *now = begin;
  while (size > 0) {
    Node_List<T> *cnt = now;
    now = now->next;
    size--;
    delete cnt;
  }
}

template<typename T>
int List<T>::GetSize() const {
  return size;
}

template<typename T>
T &List<T>::back() const {
  return end->data;
}

template<typename T>
List<T> Merge(List<T> first, List<T> second) {
  Node_List<T> *left = first.begin;
  int size1 = 0;
  Node_List<T> *right = second.begin;
  int size2 = 0;

  List<T> res;

  while (size1 < first.size and size2 < second.size) {
    if (left->data < right->data) {
      res.PushBack(left->data);
      left = left->next;
      size1++;
    } else {
      res.PushBack(right->data);
      right = right->next;
      size2++;
    }
  }
  while (size1 < first.size) {
    res.PushBack(left->data);
    size1++;
    left = left->next;
  }
  while (size2 < second.size) {
    res.PushBack(right->data);
    size2++;
    right = right->next;
  }
  return res;
}

template<typename T>
List<T> MergeSort(List<T> now) {
  if (now.size <= 1) {
    return now;
  }
  List<T> first;
  List<T> second;
  Node_List<T> *element = now.begin;
  int my_index = 0;
  while (my_index < now.size / 2) {
    first.PushBack(element->data);
    my_index++;
    element = element->next;
  }
  while (my_index < now.size) {
    second.PushBack(element->data);
    my_index++;
    element = element->next;
  }
  first = MergeSort(first);
  second = MergeSort(second);
  return Merge(first, second);
}

class Monomial {
 private:
  friend class Polynomial;

  template<typename T1>
  friend List<T1> Merge(List<T1> first, List<T1> second);

  template<typename T2>
  friend List<T2> MergeSort(List<T2> now);

  long double cf;
  std::vector<int> deg;

 public:
  Monomial() {
    deg.resize(LenAlphabet, 0);
  }

  Monomial(long double cf, std::vector<int> deg) : cf(cf),
                                                   deg(deg) {
  };

  long double GetY(std::vector<long double> variables) const;

  std::vector<int> Deg() const;

  bool operator <(Monomial second) const {
    return deg < second.deg;
  }
};

struct Comp {
  bool operator()(const Monomial &a,
                  const Monomial &b) const {
    return a.Deg() < b.Deg();
  }
};

class Polynomial {
 private:
  List<Monomial> monos;

  void Normalize();

  bool CheckCntVars() const;

 public:
  Polynomial() = default;

  Polynomial(List<Monomial> a) : monos(a) {
  };

  Polynomial(std::string s);

  long double GetY(
      std::vector<long double> variables) const;

  bool operator ==(Polynomial second);

  Polynomial operator +(Polynomial second) const;

  Polynomial operator -(Polynomial second) const;

  std::pair<bool,
            std::vector<int> > FindIntegerRoots() const;

  Polynomial operator *(Polynomial second) const;

  std::pair<Polynomial, Polynomial> operator /(Polynomial second);


  std::string GetString() const;

  Polynomial derivative(int pos);

  int GetMask() const;

  bool IsEmpty() const;
};

void DeletrSpace(std::string &s) {
  std::string cnt;
  for (auto i: s) {
    if (i != ' ') cnt += i;
  }
  s = cnt;
}

void PopFront(std::string &s) {
  std::reverse(s.begin(), s.end());
  s.pop_back();
  std::reverse(s.begin(), s.end());
}

Polynomial::Polynomial(std::string s) {
  DeletrSpace(s);
  std::string now;
  bool sign = false;
  std::map<Monomial, long double, Comp> mp;

  for (int i = 0; i < s.size(); i++) {
    if (((s[i] == '-' or s[i] == '+') and (i != 0)) or (i == s.size() - 1)) {
      if (i == s.size() - 1) {
        now += s[i];
      }
      if (now.front() == '-') {
        sign = true;
      }
      if (now.front() == '-' or now.front() == '+') {
        PopFront(now);
      }
      Monomial all;
      int pow_ten = 0;
      bool dot = false;
      all.cf = 0;
      int start = now.size();
      bool is_cf_one = false;
      for (int j = 0; j < now.size(); j++) {
        if (now[j] >= '0' and now[j] <= '9') {
          is_cf_one = true;

          if (dot) {
            all.cf += (long double) (now[j] - '0') * pow(10, pow_ten);
            pow_ten--;
          } else {
            all.cf *= 10LL;
            all.cf += (now[j] - '0');
          }
        } else if (now[j] == '.') {
          dot = true;
          pow_ten = -1;
        } else {
          start = j;
          break;
        }
      }
      if (all.cf == 0 and !is_cf_one) {
        all.cf = 1;
      }
      if (sign) {
        all.cf *= (long double) -1;
      }
      sign = false;

      int cur_pow = INF;
      for (int j = start; j < now.size(); j++) {
        int ind = j;
        if (j + 1 >= now.size() or now[j + 1] != '^') {
          cur_pow = 1;
        } else {
          cur_pow = 0;
          for (int id = j + 2; id < now.size() and now[id] >= '0' and now[id] <= '9'; id++) {
            cur_pow *= 10;
            cur_pow += now[id] - '0';
            j = id;
          }
        }
        all.deg[now[ind] - 'a'] += cur_pow;
      }
      if (mp.find(all) == mp.end()) {
        if (all.cf != 0) {
          mp[all] = monos.GetSize();
          monos.PushBack(all);
        }
      } else {
        monos[mp[all]].cf += all.cf;
      }
      now.clear();
    }
    now += s[i];
  }
  Normalize();
}

long double Monomial::GetY(std::vector<long double> variable) const {
  long double ans = cf;
  for (int j = 0; j < LenAlphabet; j++) {
    if (variable[j] != INF) {
      ans *= (long double) pow(variable[j], deg[j]);
    }
  }
  return ans;
}

std::vector<int> Monomial::Deg() const {
  return deg;
}

bool Polynomial::CheckCntVars() const {
  int mask = 0;
  int cnt = 0;
  for (int i = 0; i < monos.GetSize(); i++) {
    for (int j = 0; j < monos[i].deg.size(); j++) {
      if (monos[i].deg[j] != 0) {
        if (mask & (1 << j) == 0) {
          mask |= (1 << j);
          cnt++;
        }
      }
      if (cnt > 1) return 0;
    }
  }
  return (cnt < 2);
}

void Polynomial::Normalize() {
  std::map<Monomial, long double, Comp> temp;
  for (int i = 0; i < monos.GetSize(); i++) {
    temp[monos[i]] += monos[i].cf;
  }
  Polynomial temp2;
  for (auto j: temp) {
    if (std::abs(j.second) > EPS) {
      temp2.monos.PushBack(Monomial(j.second, j.first.deg));
    }
  }
  *this = temp2;
  this->monos = MergeSort(this->monos);
}

long double Polynomial::GetY(std::vector<long double> variables) const {
  std::vector<bool> used(LenAlphabet);
  for (int j = 0; j < monos.GetSize(); j++) {
    for (int z = 0; z < LenAlphabet; z++) {
      if (monos[j].deg[z] != 0) {
        used[z] = true;
      }
    }
  }
  long double res = 0;
  for (int j = 0; j < monos.GetSize(); j++) {
    res += monos[j].GetY(variables);
  }
  return res;
}

bool Polynomial::operator ==(Polynomial other) {
  Normalize();
  other.Normalize();
  if (monos.GetSize() != other.monos.GetSize()) {
    return false;
  }
  for (int i = 0; i < monos.GetSize(); i++) {
    if (monos[i].deg != other.monos[i].deg or
        std::abs(monos[i].cf - other.monos[i].cf) > EPS) {
      return false;
    }
  }
  return true;
}

Polynomial Polynomial::operator +(Polynomial other) const {
  std::map<Monomial, long double, Comp> cur;
  for (int i = 0; i < monos.GetSize(); i++) {
    cur[monos[i]] += monos[i].cf;
  }
  for (int i = 0; i < other.monos.GetSize(); i++) {
    cur[other.monos[i]] += other.monos[i].cf;
  }
  Polynomial res;
  for (auto i: cur) {
    if (std::abs(i.second) > EPS) {
      res.monos.PushBack(Monomial(i.second, i.first.deg));
    }
  }
  res.Normalize();
  return res;
}

Polynomial Polynomial::operator -(Polynomial other) const {
  std::map<Monomial, long double, Comp> cur;
  for (int i = 0; i < monos.GetSize(); i++) {
    cur[monos[i]] += monos[i].cf;
  }
  for (int i = 0; i < other.monos.GetSize(); i++) {
    cur[other.monos[i]] -= other.monos[i].cf;
  }
  Polynomial ans;
  for (auto i: cur) {
    if (std::abs(i.second) > EPS) {
      ans.monos.PushBack(Monomial(i.second, i.first.deg));
    }
  }
  ans.Normalize();
  return ans;
}

Polynomial Polynomial::operator *(Polynomial other) const {
  std::map<Monomial, long double, Comp> tmp;

  for (int i = 0; i < monos.GetSize(); i++) {
    for (int j = 0; j < other.monos.GetSize(); j++) {
      long double cur = other.monos[j].cf * monos[i].cf;
      std::vector<int> now_step = monos[i].deg;
      for (int k = 0; k < LenAlphabet; k++) {
        now_step[k] += other.monos[j].deg[k];
      }
      tmp[Monomial(cur, now_step)] += cur;
    }
  }
  Polynomial res;

  for (auto j: tmp) {
    if (j.second != 0) {
      res.monos.PushBack(Monomial(j.second, j.first.deg));
    }
  }
  return res;
}

std::pair<Polynomial, Polynomial> Polynomial::operator /(Polynomial other) {
  Normalize();
  other.Normalize();
  int ind = -1;
  for (int i = 0; i < other.monos.back().deg.size(); i++) {
    if (other.monos.back().deg[i])ind = i;
  }
  Polynomial cur = *this;
  Polynomial res;
  std::vector<int> deg(LenAlphabet);
  while (cur.monos.GetSize() > 0 and cur.monos.back().deg[ind] >= other.monos.back().deg[ind]) {
    deg[ind] = cur.monos.back().deg[ind] - other.monos.back().deg[ind];
    long double coef = cur.monos.back().cf / other.monos.back().cf;
    res.monos.PushBack(Monomial(coef, deg));
    Polynomial buf;
    buf.monos.PushBack(Monomial(coef, deg));
    cur = cur - (buf * other);
    cur.Normalize();
  }
  res.Normalize();
  return make_pair_custom(res, cur);
}

std::pair<bool, std::vector<int> > Polynomial::FindIntegerRoots() const {
  if (!CheckCntVars()) {
    std::vector<int> temp;
    temp.push_back(1);
    return make_pair_custom(false, temp);
  }
  int ind_var = -1;
  int ind_const = -1;
  bool good = true;

  for (int i = 0; i < monos.GetSize(); i++) {
    int cnt = 0;
    for (int j = 0; j < monos[i].deg.size(); j++) {
      if (monos[i].deg[j] != 0) {
        ind_var = j;
        cnt += monos[i].deg[j];
      }
    }
    if (cnt == 0) {
      ind_const = i;
    }
    if ((long double) ((int) (monos[i].cf)) != monos[i].cf) {
      good = false;
      break;
    }
  }

  if (ind_var == -1) {
    std::vector<int> temp;
    temp.push_back(0);
    return make_pair_custom(false, temp);
  }
  if (!good) {
    std::vector<int> temp;
    temp.push_back(2);
    return make_pair_custom(false, temp);
  }
  int val = 0;
  if (ind_const != -1) {
    val = (int) monos[ind_const].cf;
    val = abs(val);
  }
  std::vector<int> res;
  std::vector<long double> get(LenAlphabet, INF);
  int cnt = monos[ind_const].cf;
  for (int j = 1; j * j <= std::abs(cnt); j++) {
    if (std::abs(cnt) % j == 0) {
      get[ind_var] = j;
      auto tmp = GetY(get);
      if (std::abs(tmp) <= EPS) {
        res.emplace_back(get[ind_var]);
      }
      get[ind_var] = -j;
      tmp = GetY(get);
      if (std::abs(tmp) <= EPS) {
        res.emplace_back(get[ind_var]);
      }
      get[ind_var] = std::abs(cnt) / j;
      tmp = GetY(get);
      if (std::abs(tmp) <= EPS) {
        res.emplace_back(get[ind_var]);
      }
      get[ind_var] = -std::abs(cnt) / j;
      tmp = GetY(get);
      if (std::abs(tmp) <= EPS) {
        res.emplace_back(get[ind_var]);
      }
    }
  }
  get[ind_var] = 0;
  auto tmp = GetY(get);
  if (std::abs(tmp) <= EPS) {
    res.emplace_back(get[ind_var]);
  }
  std::sort(res.begin(), res.end());
  std::unique(res.begin(), res.end());

  return make_pair_custom(true, res);
}

std::string Polynomial::GetString() const {
  std::string res;
  for (int i = 0; i < monos.GetSize(); i++) {
    if (i == 0) {
      if (monos[i].cf < 0) {
        res += "- ";
      }
    } else {
      if (monos[i].cf < 0) {
        res += "- ";
      } else {
        res += "+ ";
      }
    }
    bool have_varibles = false;

    for (int j = 0; j < monos[i].deg.size(); j++) {
      if (monos[i].deg[j] != 0) {
        have_varibles = true;
      }
    }

    if ((monos[i].cf != 1 and monos[i].cf != -1) or !have_varibles) {
      std::string tmp;
      if (monos[i].cf > 0) {
        tmp = std::to_string(monos[i].cf);
      } else {
        tmp = std::to_string(-monos[i].cf);
      }

      while (tmp.size() > 0) {
        if (tmp.back() == '0') {
          tmp.pop_back();
        } else if (tmp.back() == '.') {
          tmp.pop_back();
          break;
        } else {
          break;
        }
      }
      res += tmp;
    }
    for (int j = 0; j < LenAlphabet; j++) {
      if (monos[i].deg[j] != 0) {
        res += (j + 'a');
        if (monos[i].deg[j] != 1) {
          res += '^';
          res += std::to_string(monos[i].deg[j]);
        }
      }
    }
    res += ' ';
  }
  return res;
}

int Polynomial::GetMask() const {
  int mask = 0;
  for (int i = 0; i < monos.GetSize(); i++) {
    for (int j = 0; j < monos[i].deg.size(); j++) {
      if (monos[i].deg[j] != 0) {
        mask |= (1 << j);
      }
    }
  }
  return mask;
}

bool Polynomial::IsEmpty() const {
  return monos.GetSize() == 0;
}

Polynomial Polynomial::derivative(int ind) {
  Polynomial res;
  for (int i = 0; i < monos.GetSize(); i++) {
    if (monos[i].deg[ind] != 0) {
      Monomial cnt(monos[i].cf * monos[i].deg[ind], monos[i].deg);
      cnt.deg[ind]--;
      res.monos.PushBack(cnt);
    }
  }
  res.Normalize();
  return res;
}

std::vector<std::map<char, int> > dfa(8);
//0 - Начальное состояние
//1 - После переменной
//2 - После знака ^ (ожидание степени)
//3 - В числе степени
//4 - В целой части коэффициента
//5 - После точки
//6 - В дробной части коэффициента
//7 - После знака +/-
void BuildDfa() {
  for (char c = 'a'; c <= 'z'; c++) {
    dfa[7][c] = 1;
    dfa[1][c] = 1;
    dfa[3][c] = 1;
    dfa[4][c] = 1;
    dfa[6][c] = 1;
    dfa[0][c] = 1;
  }
  for (char c = '0'; c <= '9'; c++) {
    dfa[7][c] = 4;
    dfa[2][c] = 3;
    dfa[3][c] = 3;
    dfa[4][c] = 4;
    dfa[5][c] = 6;
    dfa[6][c] = 6;
    dfa[0][c] = 4;
  }
  dfa[0]['-'] = 7;
  dfa[0]['+'] = 7;
  dfa[1]['^'] = 2;
  dfa[1]['-'] = 7;
  dfa[1]['+'] = 7;
  dfa[3]['+'] = 7;
  dfa[3]['-'] = 7;
  dfa[4]['.'] = 5;
  dfa[4]['+'] = 7;
  dfa[4]['-'] = 7;
  dfa[6]['+'] = 7;
  dfa[6]['-'] = 7;
}
void DeleteLastSpace(std::string &cur) {
  while (cur.size() and cur.back() == ' ') cur.pop_back();
}
void CheckString(int v, int ind, std::string &s) {
  DeletrSpace(s);
  if (ind == s.size()) {
    std::string res = "In position " + std::to_string(ind + 1) + ", ";
    if (v == 5) {
      res += "There are no numbers after the dot";
    } else if (v == 2) {
      res += "There are no numbers after the degree";
    } else if (v == 0 or v == 7) {
      res += "There is nothing after the sign";
    }
    if (v == 0 or v == 2 or v == 5 or v == 7) {
      throw res;
    }
    return;
  }
  if (s[ind] == ' ') {
    while (ind < s.size() and s[ind] == ' ') {
      ind++;
    }
    if (ind == s.size()) {
      return;
    } else {
      if (s[ind] >= '0' and s[ind] <= '9' and (v == 3 or v == 4 or v == 6)) {
        std::string res = "In position " + std::to_string(ind + 1) + ", you forgot a sign";
        throw res;
      }
    }
  }
  if (dfa[v].find(s[ind]) == dfa[v].end()) {
    std::string res = "Error in position " + std::to_string(ind + 1) + ", you can not use " + s[ind] + " after " + s
    [ind - 1];
    throw res;
  }
  return CheckString(dfa[v][s[ind]], ind + 1, s);
}

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <fstream>



// Use your Polynomial and List classes defined above
// Глобальный список полиномов
List<Polynomial> current;

extern const int LenAlphabet;
extern const long double INF;

// Функция запуска SFML + ImGui фронтенда вместо main
void runFrontend() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "Polynomial Calculator");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);

  sf::Clock deltaClock;

  // UI state
  static char inputBuf[256] = "";
  static char filePath[256] = "polynomials.txt";
  static int selIdxA = 0, selIdxB = 0;
  static int derivVar = 0, derivOrder = 1;
  static std::string resultString;
  static std::string errorMsg;
  static float evalValues[100] = {0.0f}; // Заменить 100 на LenAlphabet при необходимости

  // Для сохранения результатов между кадрами
  static Polynomial lastRes;
  static Polynomial lastQ, lastR;
  static bool hasLastRes = false;
  static bool hasLastQR = false;

  enum Command { None, Add, Sum, Evaluate, IntRoots, Multiply, Divide, Derivative, Compare, Delete } cmd = None;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      ImGui::SFML::ProcessEvent(event);
      if (event.type == sf::Event::Closed)
        window.close();
    }

    ImGui::SFML::Update(window, deltaClock.restart());

    // Левая панель: файл, команды и список
    ImGui::Begin("Commands & List");
    ImGui::InputText("File Path", filePath, sizeof(filePath));
    if (ImGui::Button("Save DB")) {
      std::ofstream out(filePath);
      for (int i = 0; i < current.GetSize(); ++i)
        out << current[i].GetString() << "\n";
      resultString = "Database saved.";
      hasLastRes = hasLastQR = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Load DB")) {
      std::ifstream in(filePath);
      std::string line;
      while (std::getline(in, line)) {
        try { CheckString(0, 0, line); current.PushBack(Polynomial(line)); }
        catch (...) { /* ignore */ }
      }
      resultString = "Database loaded.";
      hasLastRes = hasLastQR = false;
    }
    ImGui::Separator();
    if (ImGui::Button("Add Polynomial"))        { cmd = Add;       errorMsg.clear(); resultString.clear(); hasLastRes = hasLastQR = false; }
    if (ImGui::Button("Sum Polynomials"))      { cmd = Sum;       errorMsg.clear(); resultString.clear(); hasLastRes = hasLastQR = false; }
    if (ImGui::Button("Evaluate Value"))       { cmd = Evaluate;  errorMsg.clear(); resultString.clear(); hasLastRes = hasLastQR = false; }
    if (ImGui::Button("Integer Roots"))        { cmd = IntRoots;  errorMsg.clear(); resultString.clear(); hasLastRes = hasLastQR = false; }
    if (ImGui::Button("Multiply Polynomials")) { cmd = Multiply;  errorMsg.clear(); resultString.clear(); hasLastRes = hasLastQR = false; }
    if (ImGui::Button("Divide Polynomials"))   { cmd = Divide;    errorMsg.clear(); resultString.clear(); hasLastRes = hasLastQR = false; }
    if (ImGui::Button("Derivative"))           { cmd = Derivative;errorMsg.clear(); resultString.clear(); hasLastRes = hasLastQR = false; }
    if (ImGui::Button("Compare"))              { cmd = Compare;   errorMsg.clear(); resultString.clear(); hasLastRes = hasLastQR = false; }
    if (ImGui::Button("Delete Polynomial"))    { cmd = Delete;    errorMsg.clear(); resultString.clear(); hasLastRes = hasLastQR = false; }
    ImGui::Separator();
    ImGui::Text("Current Polynomials:");
    for (int i = 0; i < current.GetSize(); ++i) {
      std::string temp = current[i].GetString();
      std::string label = std::to_string(i) + ": " + (!temp.empty()? temp : "0");
      ImGui::Selectable(label.c_str(), false);
    }
    ImGui::End();

    // Правая панель: детали команды
    ImGui::Begin("Details");
    switch (cmd) {
      case Add: {
        ImGui::InputText("Polynomial", inputBuf, sizeof(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        if (ImGui::Button("Add")) {
          std::string s(inputBuf);
          try { CheckString(0,0,s); current.PushBack(Polynomial(s)); resultString = "Added."; inputBuf[0]='\0'; }
          catch (const std::string &e) { errorMsg = e; }
        }
        if (!errorMsg.empty()) ImGui::TextWrapped("%s", errorMsg.c_str());
        if (!resultString.empty()) ImGui::Text("%s", resultString.c_str());
        break;
      }
      case Sum: {
        ImGui::SliderInt("Index A", &selIdxA, 0, current.GetSize()-1);
        ImGui::SliderInt("Index B", &selIdxB, 0, current.GetSize()-1);
        if (ImGui::Button("Compute Sum")) {
          lastRes = current[selIdxA] + current[selIdxB];
          resultString = lastRes.GetString();
          hasLastRes = true;
        }
        if (hasLastRes && ImGui::Button("Save Result")) {
          current.PushBack(lastRes);
          resultString = "Result saved.";
          hasLastRes = false;
        }
        ImGui::TextWrapped("%s", resultString.c_str());
        break;
      }
      case Evaluate: {
        ImGui::SliderInt("Index", &selIdxA, 0, current.GetSize()-1);
        int mask = current[selIdxA].GetMask();
        for (int j = 0; j < LenAlphabet; ++j) {
          if (mask & (1<<j)) {
            char lbl[8]; snprintf(lbl, sizeof(lbl), "%c", 'a'+j);
            ImGui::InputFloat(lbl, &evalValues[j]);
          }
        }
        if (ImGui::Button("Evaluate")) {
          std::vector<long double> vals(LenAlphabet, INF);
          for (int j = 0; j < LenAlphabet; ++j)
            if (mask & (1<<j)) vals[j] = evalValues[j];
          long double r = current[selIdxA].GetY(vals);
          resultString = "Result: " + std::to_string(r);
          hasLastRes = false;
        }
        ImGui::TextWrapped("%s", resultString.c_str());
        break;
      }
      case IntRoots: {
        ImGui::SliderInt("Index", &selIdxA, 0, current.GetSize()-1);
        if (ImGui::Button("Find Roots")) {
          auto pr = current[selIdxA].FindIntegerRoots();
          resultString = pr.first ? "Roots:" : "None";
          for (auto r : pr.second) resultString += " " + std::to_string(r);
          hasLastRes = false;
        }
        ImGui::TextWrapped("%s", resultString.c_str());
        break;
      }
      case Multiply: {
        ImGui::SliderInt("Index A", &selIdxA, 0, current.GetSize()-1);
        ImGui::SliderInt("Index B", &selIdxB, 0, current.GetSize()-1);
        if (ImGui::Button("Multiply")) {
          lastRes = current[selIdxA] * current[selIdxB];
          resultString = lastRes.GetString();
          hasLastRes = true;
        }
        if (hasLastRes && ImGui::Button("Save Result")) {
          current.PushBack(lastRes);
          resultString = "Result saved.";
          hasLastRes = false;
        }
        ImGui::TextWrapped("%s", resultString.c_str());
        break;
      }
      case Divide: {
        ImGui::SliderInt("Dividend", &selIdxA, 0, current.GetSize()-1);
        ImGui::SliderInt("Divisor", &selIdxB, 0, current.GetSize()-1);
        if (ImGui::Button("Divide")) {
          int maskA = current[selIdxA].GetMask();
          int maskB = current[selIdxB].GetMask();
          std::cout << maskB;
          if (maskB == 0){
            resultString = "Division by zero is incorrect.";
            hasLastQR = false;
          }
          else if ((maskA & (maskA-1)) || (maskB & (maskB-1))) {
            resultString = "Division only for univariate polynomials.";
            hasLastQR = false;
          } else {
            auto qr = current[selIdxA] / current[selIdxB];
            lastQ = qr.first;
            lastR = qr.second;
            resultString = "Q:" + (!lastQ.GetString().empty()? lastQ.GetString() : "0")  + " R:" + (!lastR.GetString().empty()? lastQ.GetString() : "0");
            hasLastQR = true;
          }
        }
        if (hasLastQR) {
          if (ImGui::Button("Save Quotient")) { current.PushBack(lastQ); resultString = "Quotient saved."; hasLastQR = false; }
          ImGui::SameLine();
          if (ImGui::Button("Save Remainder")) { current.PushBack(lastR); resultString = "Remainder saved."; hasLastQR = false; }
        }
        ImGui::TextWrapped("%s", resultString.c_str());
        break;
      }
      case Derivative: {
        ImGui::SliderInt("Index", &selIdxA, 0, current.GetSize()-1);
        ImGui::InputInt("Variable (0=a,...)", &derivVar);
        ImGui::InputInt("Order", &derivOrder);
        if (ImGui::Button("Derive")) {
          lastRes = current[selIdxA];
          for (int i = 0; i < derivOrder; ++i) lastRes = lastRes.derivative(derivVar);
          resultString = lastRes.GetString();
          hasLastRes = true;
        }
        if (hasLastRes && ImGui::Button("Save Result")) {
          current.PushBack(lastRes);
          resultString = "Result saved.";
          hasLastRes = false;
        }
        ImGui::TextWrapped("%s", resultString.c_str());
        break;
      }
      case Compare: {
        ImGui::SliderInt("A", &selIdxA, 0, current.GetSize()-1);
        ImGui::SliderInt("B", &selIdxB, 0, current.GetSize()-1);
        if (ImGui::Button("Compare")) {
          resultString = (current[selIdxA] == current[selIdxB]) ? "Equal" : "Not equal";
          hasLastRes = false;
        }
        ImGui::TextWrapped("%s", resultString.c_str());
        break;
      }
      case Delete: {
        ImGui::SliderInt("Index to delete", &selIdxA, 0, current.GetSize()-1);
        if (ImGui::Button("Delete")) {
          current.Erase(selIdxA); // реализуйте метод RemoveAt или аналогичный
          resultString = "Deleted.";
          selIdxA = 0; // сброс
        }
        ImGui::TextWrapped("%s", resultString.c_str());
        break;
      }
      default:
        ImGui::Text("Select a command on the left.");
    }
    ImGui::End();

    window.clear(sf::Color(15,15,15));
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}




int main(){
  BuildDfa();
  runFrontend();
  return 0;
}
