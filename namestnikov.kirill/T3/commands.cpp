#include "commands.hpp"
#include <algorithm>
#include <iomanip>
#include <functional>
#include <numeric>
#include <stream_guard.hpp>
#include "polygon.hpp"

bool isPolygon(const namestnikov::Polygon & polygon)
{
  return (polygon.points.size() > 2);
}

bool isEven(const namestnikov::Polygon & polygon)
{
  return (isPolygon(polygon)) && (polygon.points.size() % 2 == 0);
}

bool isOdd(const namestnikov::Polygon & polygon)
{
  return !isEven(polygon);
}

bool isProperSize(const namestnikov::Polygon & polygon, size_t number)
{
  return (polygon.points.size() == number);
}

void namestnikov::getCount(const std::vector< Polygon > & data, std::istream & in, std::ostream & out)
{
  std::string argument = "";
  in >> argument;
  if (argument == "EVEN")
  {
    out << std::count_if(data.begin(), data.end(), isEven);
  }
  else if (argument == "ODD")
  {
    out << std::count_if(data.begin(), data.end(), isOdd);
  }
  else
  {
    size_t pointsCount = std::stoull(argument);
    if (pointsCount < 3)
    {
      throw std::logic_error("Wrong number of points");
    }
    using namespace std::placeholders;
    std::function< bool(const Polygon &) > isRightCount = std::bind(isProperSize, _1, pointsCount);
    out << std::count_if(data.begin(), data.end(), isRightCount);
  }
}

void namestnikov::getArea(const std::vector< Polygon > & data, std::istream & in, std::ostream & out)
{
  StreamGuard guard(out);
  out << std::setprecision(1) << std::fixed;
  std::string argument = "";
  in >> argument;
  std::vector< Polygon > shapes;
  using namespace std::placeholders;
  if (argument == "EVEN")
  {
    std::copy_if(data.cbegin(), data.cend(), std::back_inserter(shapes), isEven);
  }
  else if (argument == "ODD")
  {
    std::copy_if(data.cbegin(), data.cend(), std::back_inserter(shapes), isOdd);
  }
  else if (argument ==  "MEAN")
  {
    if (data.empty())
    {
      throw std::logic_error("No shapes to accumulate area");
    }
    std::copy(data.cbegin(), data.cend(), std::back_inserter(shapes));
  }
  else
  {
    size_t pointsCount = std::stoull(argument);
    if (pointsCount < 3)
    {
      throw std::logic_error("Wrong number of points");
    }
    std::function< bool(const Polygon &) > isRightCount = std::bind(isProperSize, _1, pointsCount);
    std::copy_if(data.cbegin(), data.cend(), std::back_inserter(shapes), isRightCount);
  }
  std::vector< double > areas;
  std::transform(shapes.cbegin(), shapes.cend(), std::back_inserter(areas), getPolygonArea);
  double result = std::accumulate(areas.cbegin(), areas.cend(), 0.0);
  out << (argument == "MEAN") ? (result / data.size()) : result;
}

bool comparePoints(const namestnikov::Polygon & first, const namestnikov::Polygon & second)
{
  return (first.points.size() < second.points.size());
}

bool compareArea(const namestnikov::Polygon & first, const namestnikov::Polygon & second)
{
  return (getPolygonArea(first) < getPolygonArea(second));
}

void namestnikov::getMax(const std::vector< Polygon > & data, std::istream & in, std::ostream & out)
{
  std::string argument = "";
  in >> argument;
  if (data.empty())
  {
    throw std::logic_error("No polygons for this command");
  }
  else
  {
    if (argument == "AREA")
    {
      StreamGuard guard(out);
      out << std::setprecision(1) << std::fixed;
      out << getPolygonArea(*std::max_element(data.begin(), data.end(), compareArea));
    }
    else if (argument == "VERTEXES")
    {
      out << std::max_element(data.begin(), data.end(), comparePoints)->points.size();
    }
    else
    {
      throw std::logic_error("Wrong argument");
    }
  }
}

void namestnikov::getMin(const std::vector< Polygon > & data, std::istream & in, std::ostream & out)
{
  std::string argument = "";
  in >> argument;
  if (data.empty())
  {
    throw std::logic_error("No polygons for this command");
  }
  else
  {
    if (argument == "AREA")
    {
      StreamGuard guard(out);
      out << std::setprecision(1) << std::fixed;
      out << getPolygonArea(*std::min_element(data.begin(), data.end(), compareArea));
    }
    else if (argument == "VERTEXES")
    {
      out << std::min_element(data.begin(), data.end(), comparePoints)->points.size();
    }
    else
    {
      throw std::logic_error("Wrong argument");
    }
  }
}

bool hasIntersection(const namestnikov::Polygon & first, const namestnikov::Polygon & second)
{
  auto left = std::minmax_element(first.points.begin(), first.points.end());
  auto right = std::minmax_element(second.points.begin(), second.points.end());
  bool check = (*left.second >= *right.first) && (*left.first <= *right.second);
  check = check || ((*right.second >= *left.first) && (*right.first <= *left.second));
  return check;
}

void namestnikov::getIntersections(const std::vector< Polygon > & data, std::istream & in, std::ostream & out)
{
  Polygon polygon;
  in >> polygon;
  if ((polygon.points.empty()) || (!in) || (in.peek() != '\n'))
  {
    throw std::logic_error("Wrong argument");
  }
  using namespace std::placeholders;
  auto isIntersected = std::bind(hasIntersection, std::cref(polygon), _1);
  out << std::count_if(data.begin(), data.end(), isIntersected);
}

bool isSamePolygon(const namestnikov::Polygon & first, const namestnikov::Polygon & second)
{
  bool check = true;
  check = check && (first.points.size() == second.points.size());
  check = check && (std::equal(first.points.cbegin(), first.points.cend(), second.points.cbegin()));
  return check;
}

namestnikov::Polygon namestnikov::EchoMaker::operator()(const Polygon & other)
{
  if (other == target)
  {
    data.push_back(target);
  }
  return other;
}

void namestnikov::getEcho(std::vector< Polygon > & data, std::istream & in, std::ostream & out)
{
  Polygon polygon;
  in >> polygon;
  if ((polygon.points.empty()) || (!in) || (in.peek() != '\n'))
  {
    throw std::logic_error("Wrong argument");
  }
  using namespace std::placeholders;
  auto isSame = std::bind(isSamePolygon, std::cref(polygon), _1);
  out << std::count_if(data.begin(), data.end(), isSame);
  std::vector< Polygon > temp;
  EchoMaker maker{temp, polygon};
  std::transform(std::make_move_iterator(data.begin()), std::make_move_iterator(data.end()), std::back_inserter(temp), std::ref(maker));
  data = temp;
}