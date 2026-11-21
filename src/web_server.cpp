#include "web_server.h"
#include <httplib.h>
#include <cstdlib>
#include <sstream>
#include <unordered_map>

static std::unordered_map<std::string, std::string> form(const std::string &body)
{
    std::unordered_map<std::string, std::string> values;
    size_t start = 0;
    while (start < body.size())
    {
        size_t end = body.find('&', start);
        if (end == std::string::npos)
            end = body.size();
        auto pair = body.substr(start, end - start);
        auto eq = pair.find('=');
        if (eq != std::string::npos)
        {
            std::string key = pair.substr(0, eq), value = pair.substr(eq + 1);
            auto unescape = [](std::string s)
            {
                std::string out;
                for (size_t i = 0; i < s.size(); ++i)
                {
                    if (s[i] == '+')
                        out += ' ';
                    else if (s[i] == '%' && i + 2 < s.size())
                    {
                        char hex[3] = {s[i + 1], s[i + 2], 0};
                        out += char(strtol(hex, nullptr, 16));
                        i += 2;
                    }
                    else
                        out += s[i];
                }
                return out;
            };
            values[unescape(key)] = unescape(value);
        }
        start = end + 1;
    }
    return values;
}

static std::string escapeJson(const std::string &s)
{
    std::ostringstream o;
    for (char c : s)
    {
        if (c == '\\' || c == '"')
            o << '\\' << c;
        else if (c == '\n')
            o << "\\n";
        else
            o << c;
    }
    return o.str();
}

WebServer::WebServer(Database &database, int port) : database_(database), port_(port) {}

void WebServer::run()
{
    httplib::Server server;

    server.Get("/api/health", [](const httplib::Request &, httplib::Response &res)
               { res.set_content("{\"status\":\"ok\"}", "application/json"); });

    server.Get("/api/drivers", [this](const httplib::Request &, httplib::Response &res)
               {
    std::ostringstream o; o<<"[";
    auto list=database_.drivers();
    for(size_t i=0;i<list.size();++i){
      const auto& d=list[i]; if(i)o<<",";
      o<<"{\"id\":"<<d.id<<",\"stageName\":\""<<escapeJson(d.stageName)
       <<"\",\"name\":\""<<escapeJson(d.name)<<"\",\"phone\":\""<<escapeJson(d.phone)
       <<"\",\"plate\":\""<<escapeJson(d.plate)<<"\",\"isActive\":"<<(d.isActive?"true":"false")<<"}";
    }
    o<<"]"; res.set_content(o.str(),"application/json"); });

    server.Post("/api/drivers", [this](const httplib::Request &req, httplib::Response &res)
                {
    try {
      auto v=form(req.body); Driver d;
      d.stageName=v["stage_name"]; d.name=v["name"]; d.phone=v["phone"]; d.plate=v["plate"];
      if(d.stageName.empty()||d.name.empty()||d.phone.empty()||d.plate.empty()) throw std::runtime_error("All fields required");
      database_.addDriver(d); res.status=201;
      res.set_content("{\"status\":\"created\"}","application/json");
    } catch(const std::exception& e) { res.status=400; res.set_content(std::string("{\"error\":\"")+escapeJson(e.what())+"\"}","application/json"); } });

    server.Post("/api/visits", [this](const httplib::Request &req, httplib::Response &res)
                {
    try {
      auto v=form(req.body); 
      const std::string status=v["status"];
      if(status != "checked_in" && status != "checked_out") throw std::runtime_error("Invalid status");
      database_.recordStageVisit(std::stol(v["driver_id"]), status);
      res.set_content("{\"status\":\"recorded\"}","application/json");
    } catch(const std::exception& e) { res.status=400; res.set_content(std::string("{\"error\":\"")+escapeJson(e.what())+"\"}","application/json"); } });

    server.Get("/api/records", [this](const httplib::Request &, httplib::Response &res)
               {
    std::ostringstream o;o<<"["; auto list=database_.records();
    for(size_t i=0;i<list.size();++i){const auto&r=list[i];if(i)o<<",";
      o<<"{\"id\":"<<r.id<<",\"driverId\":"<<r.driverId<<",\"stageName\":\""<<escapeJson(r.stageName)<<"\",\"status\":\""<<escapeJson(r.status)<<"\",\"recordedAt\":\""<<escapeJson(r.recordedAt)<<"\"}";
    } o<<"]"; res.set_content(o.str(),"application/json"); });

    server.Get("/", [](const httplib::Request &, httplib::Response &res)
               {
    static const char* html = R"HTML(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>Boda Stage Manager</title>
<style>:root{--green:#0b8f5a;--ink:#18332a}*{box-sizing:border-box}body{margin:0;font-family:system-ui;background:#eef6f2;color:var(--ink)}header{background:var(--green);color:white;padding:22px 5%}.shell{max-width:1100px;margin:auto;padding:24px}.grid{display:grid;grid-template-columns:360px 1fr;gap:18px}.card{background:white;border-radius:14px;padding:18px;box-shadow:0 6px 20px #1c4b3912}input,select,button{width:100%;padding:11px;margin:6px 0;border-radius:8px;border:1px solid #cbdad3}button{background:var(--green);color:white;border:0;font-weight:700;cursor:pointer}.actions{display:flex;gap:8px}.actions button{width:auto;flex:1}table{width:100%;border-collapse:collapse}th,td{text-align:left;padding:10px;border-bottom:1px solid #edf2ef}.badge{background:#dff4e9;color:#087648;padding:4px 8px;border-radius:999px}@media(max-width:800px){.grid{grid-template-columns:1fr}}</style></head>
<body><header><h1>Boda Boda Stage Manager</h1></header><main class="shell"><div class="grid"><section class="card"><h2>Register Driver</h2>
<form id="driverForm"><input name="stage_name" placeholder="Stage name" required><input name="name" placeholder="Full name" required><input name="phone" placeholder="Phone number" required><input name="plate" placeholder="Plate / motorcycle number" required><button>Add driver</button></form></section>
<section class="card"><h2>Drivers</h2><div style="overflow:auto"><table><thead><tr><th>Name</th><th>Stage</th><th>Plate</th><th>Action</th></tr></thead><tbody id="drivers"></tbody></table></div></section></div>
<section class="card" style="margin-top:18px"><h2>Recent stage activity</h2><div style="overflow:auto"><table><thead><tr><th>Stage</th><th>Driver ID</th><th>Status</th><th>Time</th></tr></thead><tbody id="records"></tbody></table></div></section></main>
<script>
async function refresh(){let ds=await(await fetch('/api/drivers')).json();document.querySelector('#drivers').innerHTML=ds.map(d=>`<tr><td>${d.name}</td><td>${d.stageName}</td><td>${d.plate}</td><td><div class="actions"><button onclick="visit(${d.id},'checked_in')">In</button><button onclick="visit(${d.id},'checked_out')" style="background:#d97706">Out</button></div></td></tr>`).join('')||'<tr><td colspan="4">No drivers yet</td></tr>';let rs=await(await fetch('/api/records')).json();document.querySelector('#records').innerHTML=rs.map(r=>`<tr><td>${r.stageName}</td><td>${r.driverId}</td><td><span class="badge">${r.status}</span></td><td>${r.recordedAt}</td></tr>`).join('')||'<tr><td colspan="4">No activity yet</td></tr>'}
document.querySelector('#driverForm').onsubmit=async e=>{e.preventDefault();let r=await fetch('/api/drivers',{method:'POST',body:new URLSearchParams(new FormData(e.target)),headers:{'Content-Type':'application/x-www-form-urlencoded'}});if(r.ok){e.target.reset();refresh()}else alert(await r.text())};async function visit(id,status){await fetch('/api/visits',{method:'POST',body:new URLSearchParams({driver_id:id,status}),headers:{'Content-Type':'application/x-www-form-urlencoded'}});refresh()}refresh();
</script></body></html>)HTML";
    res.set_content(html, "text/html"); });

    server.listen("0.0.0.0", port_);
}
