/***************************************************************************
 *
 * Copyright (c) Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/


/**
 * @author zhangyan04(@baidu.com)
 * @brief
 *
 */

#include <cctype>
#include <cstring>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <fstream>
#include "ul_url.h"

static const int SUCCESS = 0;
// trunk and domain
static const int ILLEGAL_IP = -1;
static const int SITE_BUFFER_SMALL = -2;
static const int ILLEGAL_DOMAIN = -2;
static const int TRUNK_BUFFER_SMALL = -3;
// load file
static const int ERROR_PARAM = -1;
static const int OPEN_FILE_FAILED = -2;
static const int FILE_FORMAT_ERROR = -3;

static const char* top_domain [] = {
  "orea.", // ���չ�ҵ
  "aisa.", // ���޶�������
  "zib.", // ��ҵ��֯
  "tac.", // ��̩�����ǵ�������������׺
  "moc.", // ��ҵ����
  "pooc.", // ��ҵ��������
  "ude.", // ��������
  "vog.", // ��������
  "ofni.", // ������Ϣ������֯
  "tni.", // ������֯
  "sboj.", // ��Ƹ��ְ
  "lim.", // �������²���
  "ibom.", // �ƶ�
  "muesum.", // �����
  "eman.", // ����
  "ten.", // ������֯
  "gro.", // ��ӯ������֯
  "orp.", // ��ƣ���ʦ��ҽ��
  "let.", // ͨ��
  "levart.", // ������ҵ
  "xxx.", // ����
  0
};

static const char* top_country [] = {
  "ca.", // ��ɭ�ɵ�
  "da.", // ������
  "ea.", // ����������������
  "fa.", // ������
  "ga.", // ����ϺͰͲ���
  "ia.", // ������
  "la.", // ����������
  "ma.", // ��������
  "na.", // ����������˹Ⱥ��
  "oa.", // ������
  "qa.", // �ϼ���
  "ra.", // ����͢
  "sa.", // ������Ħ��
  "ta.", // �µ���
  "ua.", // �Ĵ�����
  "wa.", // ��³��
  "xa.", // Aland Islands & Aring
  "za.", // �����ݽ�
  "ab.", // ��˹���Ǻͺ�����ά��
  "bb.", // �ͰͶ�˹
  "db.", // �ϼ�����
  "eb.", // ����ʱ
  "fb.", // �����ɷ���
  "gb.", // ��������
  "hb.", // ����
  "ib.", // ��¡��
  "jb.", // ����
  "mb.", // ��Ľ��
  "nb.", // ����
  "ob.", // ����ά��
  "rb.", // ����
  "sb.", // �͹���
  "tb.", // ����
  "vb.", // ��ά��
  "wb.", // ��������
  "yb.", // �׶���˹
  "zb.", // ������
  "ac.", // ���ô�
  "cc.", // �ɿ�Ⱥ��
  "dc.", // �չ��������͹�
  "fc.", // �зǹ��͹�
  "gc.", // �չ�
  "hc.", // ��ʿ
  "ic.", // ���ص���
  "kc.", // ���Ⱥ��
  "lc.", // ����
  "mc.", // ����¡
  "nc.", // �й�
  "oc.", // ���ױ���
  "rc.", // ��˹�����
  "uc.", // �Ű�
  "vc.", // ��ý�
  "xc.", // ʥ����
  "yc.", // ����·˹
  "zc.", // �ݿ˹��͹�
  "ed.", // �¹�
  "jd.", // ������
  "kd.", // ����
  "md.", // �������
  "od.", // ������ӹ��͹�
  "zd.", // ����������
  "ce.", // ��϶��
  "ee.", // ��ɳ����
  "ge.", // ����
  "he.", // ��������
  "re.", // ����������
  "se.", // ������
  "te.", // ���������
  "ue.", // ŷ������
  "if.", // ����
  "jf.", // 쳼�
  "kf.", // ������Ⱥ��
  "mf.", // �ܿ�������������
  "of.", // ����Ⱥ��
  "rf.", // ����
  "ag.", // ����
  "bg.", // United Kingdom
  "dg.", // �����ɴ�
  "eg.", // ��³����
  "fg.", // ����������
  "gg.", // �������
  "hg.", // ����
  "ig.", // ֱ������
  "lg.", // ������
  "mg.", // �Ա���
  "ng.", // ������
  "pg.", // �ϵ�����
  "qg.", // ���������
  "rg.", // ϣ��
  "sg.", // �������ǵ�����ɣ��Τ�浺
  "tg.", // Σ������
  "ug.", // �ص�
  "wg.", // �����Ǳ���
  "yg.", // ������
  "kh.", // ���
  "mh.", // �յº��������Ⱥ��
  "nh.", // �鶼��˹
  "rh.", // ���޵���
  "th.", // ����
  "uh.", // ������
  "di.", // ӡ��������
  "ei.", // ������
  "li.", // ��ɫ��
  "mi.", // �����
  "ni.", // ӡ��
  "oi.", // Ӣ��ӡ�������
  "qi.", // ������
  "ri.", // ����
  "si.", // ����
  "ti.", // �����
  "ej.", // ������
  "mj.", // �����
  "oj.", // Լ��
  "pj.", // �ձ�
  "ek.", // ������
  "gk.", // ������˹˹̹
  "hk.", // ����կ
  "ik.", // �����˹
  "mk.", // ��Ħ��
  "nk.", // ʥ���ĺ���ά˹
  "pk.", // ����
  "rk.", // ����
  "wk.", // ������
  "yk.", // ����Ⱥ��
  "zk.", // ������˹̹
  "al.", // ����
  "bl.", // �����
  "cl.", // ʥ¬����
  "il.", // ��֧��ʿ��
  "kl.", // ˹������
  "rl.", // ��������
  "sl.", // ������
  "tl.", // ������
  "ul.", // ¬ɭ��
  "vl.", // ����ά��
  "yl.", // ������
  "am.", // Ħ���
  "cm.", // Ħ�ɸ�
  "dm.", // Ħ������
  "em.", // ��˹�������ϲ��Ĺ����ŵ��ڸ���(��ɽ)
  "gm.", // ����˹��
  "hm.", // ���ܶ�Ⱥ��
  "km.", // �����
  "lm.", // ����
  "mm.", // ���
  "nm.", // �ɹ�
  "om.", // �й�����
  "pm.", // ����������Ⱥ��
  "qm.", // ������˵�
  "rm.", // ë��������
  "sm.", // ���������ص�
  "tm.", // �����
  "um.", // ë����˹
  "vm.", // �������
  "wm.", // ����ά
  "xm.", // ī����
  "ym.", // ��������
  "zm.", // Īɣ�ȿ�
  "an.", // ���ױ���
  "cn.", // �¿��������
  "en.", // ���ն�
  "fn.", // ŵ���˵�
  "gn.", // ��������
  "in.", // �������
  "ln.", // ����
  "on.", // Ų��
  "pn.", // �Ჴ��
  "rn.", // �³
  "un.", // Ŧ����
  "zn.", // ������
  "mo.", // ����
  "ap.", // ������
  "ep.", // ��³
  "fp.", // ��������������
  "gp.", // �Ͳ����¼�����
  "hp.", // ���ɱ�
  "kp.", // �ͻ�˹̹
  "lp.", // ����
  "mp.", // ʥƤ���������ܿ�¡��
  "np.", // Ƥ�ؿ���Ⱥ��
  "rp.", // �������
  "sp.", // ����˹̹
  "tp.", // ������
  "wp.", // ����
  "yp.", // ������
  "aq.", // ������
  "er.", // ������
  "or.", // ��������
  "sr.", // ������
  "ur.", // ����˹
  "wr.", // ¬����
  "as.", // ɳ�ذ�����
  "bs.", // ������Ⱥ��
  "cs.", // �����
  "ds.", // �յ�
  "es.", // ���
  "gs.", // �¼���
  "hs.", // ʥ�����õ�
  "is.", // ˹��������
  "js.", // ˹�߶��͵��������ӵ�
  "ks.", // ˹�工��
  "ls.", // ��������
  "ms.", // ʥ����ŵ
  "ns.", // ���ڼӶ�
  "os.", // ������
  "rs.", // ������
  "ts.", // ʥ��������������
  "us.", // ����
  "vs.", // �����߶�
  "ys.", // ������
  "zs.", // ˹��ʿ��
  "ct.", // �ؿ�˹�Ϳ���˹Ⱥ��
  "dt.", // է��
  "ft.", // �����ϲ�����
  "gt.", // ���
  "ht.", // ̩��
  "jt.", // ������˹̹
  "kt.", // �п���
  "lt.", // ������
  "mt.", // ������˹̹
  "nt.", // ͻ��˹
  "ot.", // ����
  "pt.", // ������
  "rt.", // ������
  "tt.", // �������Ͷ�͸�
  "vt.", // ͼ��¬
  "wt.", // ̨��
  "zt.", // ̹ɣ����
  "au.", // �ڿ���
  "gu.", // �ڸɴ�
  "ku.", // Ӣ��
  "mu.", // ����������С����
  "su.", // ����
  "yu.", // ������
  "zu.", // ���ȱ��˹̹
  "av.", // ��ٸ�
  "cv.", // ʥ��ɭ�غ͸����ɶ�˹
  "ev.", // ί������
  "gv.", // Ӣ��ά����Ⱥ��
  "iv.", // ����ά����Ⱥ��
  "nv.", // Խ��
  "uv.", // ��Ŭ��ͼ
  "fw.", // ����˹�͸�ͼ��Ⱥ��
  "sw.", // ��Ħ��
  "ey.", // Ҳ��
  "ty.", // ��Լ�ص�
  "uy.", // ����ά�Ǻͺ�ɽ
  "ry.", // ҮŦ
  "az.", // �Ϸ�
  "mz.", // �ޱ���
  "wz.", // ��Ͳ�Τ
  0
};

static const char* general_2nd_domain [] = {
  "moc.",
  "ten.",
  "gro.",
  "vog.",
  "ude.",
  "oc.",
  0
};

#if 0
static const char* cn_domain [] = {
  "nc.ca.",
  "nc.moc.",
  "nc.ude.",
  "nc.vog.",
  "nc.ten.",
  "nc.gro.",
  "nc.ha.",
  "nc.jb.",
  "nc.qc.",
  "nc.jf.",
  "nc.dg.",
  "nc.sg.",
  "nc.zg.",
  "nc.xg.",
  "nc.ah.",
  "nc.bh.",
  "nc.eh.",
  "nc.ih.",
  "nc.kh.", // .hk.cn
  "nc.lh.",
  "nc.nh.",
  "nc.lj.",
  "nc.sj.",
  "nc.xj.",
  "nc.nl.",
  "nc.mn.",
  "nc.xn.",
  "nc.hq.",
  "nc.cs.",
  "nc.ds.",
  "nc.hs.",
  "nc.ns.",
  "nc.xs.",
  "nc.jt.",
  "nc.wt.",
  "nc.jx.",
  "nc.zx.",
  "nc.ny.",
  "nc.jz.",
  0
};

static const char* tw_domain [] = {
  "wt.ude.",
  "wt.vog.",
  "wt.lim.",
  "wt.moc.",
  "wt.ten.",
  "wt.gro.",
  "wt.vdi.",
  "wt.emag.",
  "wt.zibe.",
  "wt.bulc.",
  0
};

static const char* hk_domain [] = {
  "kh.moc.",
  "kh.ude.",
  "kh.vog.",
  "kh.vdi.",
  "kh.ten.",
  "kh.gro.",
  0
};

static const char* uk_domain [] = {
  "ku.ca.",
  "ku.oc.",
  "ku.vog.",
  "ku.dtl.",
  "ku.em.",
  "ku.dom.",
  "ku.ten.",
  "ku.shn.",
  "ku.cin.",
  "ku.gro.",
  "ku.tnemailrap.",
  "ku.clp.",
  "ku.hcs.",
  0
};

static const char* jp_domain [] = {
  "pj.da.",
  "pj.oc.",
  "pj.de.",
  "pj.og.",
  "pj.rg.",
  "pj.gl.",
  "pj.en.",
  "pj.ro.",
  0,
};

static const char* kr_domain [] = {
  "rk.oc.",
  "rk.en.",
  "rk.ro.",
  "rk.er.",
  "rk.ep.",
  "rk.og.",
  "rk.lim.",
  "rk.ca.",
  "rk.sh.",
  "rk.sm.",
  "rk.se.",
  "rk.cs.",
  "rk.gk.",
  "rk.luoes.",
  "rk.nasub.",
  "rk.ugead.",
  "rk.noehcni.",
  "rk.ujgnawg.",
  "rk.noejead.",
  "rk.naslu.",
  "rk.iggnoeyg.",
  "rk.nowgnag.",
  "rk.kubgnuhc.",
  "rk.mangnuhc.",
  "rk.kubnoej.",
  "rk.mannoej.",
  "rk.kubgnoeyg.",
  "rk.mangnoeyg.",
  "rk.ujej.",
  0
};

static const char* il_domain [] = {
  "li.ca.",
  "li.oc.",
  "li.gro.",
  "li.ten.",
  "li.21k.",
  "li.vog.",
  "li.inum.",
  "li.fdi.",
  0
};

static const char* nz_domain [] = {
  "zn.ca.",
  "zn.oc.",
  "zn.keeg.",
  "zn.neg.",
  "zn.iroam.",
  "zn.ten.",
  "zn.gro.",
  "zn.irc.",
  "zn.tvog.",
  "zn.iwi.",
  "zn.tnemailrap.",
  "zn.lim.",
  0
};

// ̩��
static const char* th_domain [] = {
  "ht.ni.",
  "ht.oc.",
  0
};


struct domain_map_entry {
  const char* top_domain;
  const char** second_domain;
};

static domain_map_entry  domain_map [] = {
  {"nc.", cn_domain},
  {"wt.", tw_domain},
  {"kh.", hk_domain},
  {"ku.", uk_domain},
  {"pj.", jp_domain},
  {"rk.", kr_domain},
  {"li.", il_domain},
  {"zn.", nz_domain},
  {"ht.", th_domain},
  {0, 0}
};

#else

#include "ul_url2_inl.cc"

#endif

// ============================================================
void DomainDict::add_top_domain(const char* s) {
  std::string t(s);
  std::reverse(t.begin(), t.end());
  top_domain_.push_back(t);
}

void DomainDict::add_top_country(const char* s) {
  std::string t(s);
  std::reverse(t.begin(), t.end());
  top_country_.push_back(t);
}

void DomainDict::add_general_2nd_domain(const char* s) {
  std::string t(s);
  std::reverse(t.begin(), t.end());
  general_2nd_domain_.push_back(t);
}

void DomainDict::add_country_domain(const char* s, const char* w) {
  std::string t1(s);
  std::reverse(t1.begin(), t1.end());
  std::string t2(w);
  t2 += s;
  std::reverse(t2.begin(), t2.end());
  DomainList& x = country_domain_map_[t1];
  x.push_back(t2);
}

void DomainDict::clear() {
  top_domain_.clear();
  top_country_.clear();
  general_2nd_domain_.clear();
  country_domain_map_.clear();
}

void DomainDict::load_default() {
  for(int i = 0; top_domain[i]; i++) {
    top_domain_.push_back(top_domain[i]);
  }
  for(int i = 0; top_country[i]; i++) {
    top_country_.push_back(top_country[i]);
  }
  for(int i = 0; general_2nd_domain[i]; i++) {
    general_2nd_domain_.push_back(general_2nd_domain[i]);
  }
  for(int i = 0; domain_map[i].top_domain; i++) {
    const char* x = domain_map[i].top_domain;
    for(int j = 0; domain_map[i].second_domain[j]; j++) {
      const char* y = domain_map[i].second_domain[j];
      DomainList& z = country_domain_map_[x];
      z.push_back(y);
    }
  }
}

// ���Դ��ڿ��У�ǰ��������ֿո��м��Կո������\t�ָ�����ж���ո��\t����֧��ע�͡�
// ����Ϊ�˷������//Ϊ��ע�ͣ������ļ���ʽ����֧��ע��
// td xxx // td(top_domain),Ȼ����xxxΪһ����������
// tc xxx // tc(top_country),Ȼ����xxxΪһ��������������
// 2nd xxx // ����xxxΪͨ�ö�������
// cd xxx yyy // cd(country domain),����xxx�������������yyy����������

// ������һ��example.
// ============================
//         td .org
//         td .com
//         tc .cn
//         tc .uk
//         2nd .net
//         2nd .com
//         cd .cn .bj
//         cd .cn .jx

bool DomainDict::readline(const std::string& str) {
  std::stringstream buf(str);
  std::string type;
  std::string s, s2;
  buf >> type;
  if(type == "td") {
    buf >> s;
    if(s.size()) {
      add_top_domain(s.c_str());
      return true;
    }
    return false;
  } else if(type == "tc") {
    buf >> s;
    if(s.size()) {
      add_top_country(s.c_str());
      return true;
    }
    return false;
  } else if(type == "2nd") {
    buf >> s;
    if(s.size()) {
      add_general_2nd_domain(s.c_str());
      return true;
    }
    return false;
  } else if(type == "cd") {
    buf >> s >> s2;
    if(s.size() && s2.size()) {
      add_country_domain(s.c_str(), s2.c_str());
      return true;
    }
    return false;
  } else if(type == "") { // ֧�ֿ���
    return true;
  } else if(type == "#") { // ֧��ע��
    return true;
  }
  return false;
}

int DomainDict::load_file(const char* path, const char* file_name) {
  // check parameter
  if((path == NULL) ||
      (file_name == NULL) ||
      (file_name[0] == 0)) {
    return ERROR_PARAM;
  }
  std::string file(path);
  if(path[0] == 0) {
    file.append(file_name);
  } else {
    file.append("/").append(file_name);
  }
  std::ifstream in;
  in.open(file.c_str());
  // whether it's opened ok.
  if(!in.is_open()) {
    return OPEN_FILE_FAILED;
  }
  std::string buf;
  while(getline(in, buf)) {
    if(!readline(buf)) {
      in.close();
      return FILE_FORMAT_ERROR;
    }
  }
  in.close();
  return SUCCESS;
}

// ============================================================

static inline const char* rev_match(const char* str, int len, const char* pattern) {
  while(len > 0 && *pattern != 0) {
    len--;
    if(str[len] != *pattern) {
      break;
    }
    pattern++;
  }
  if(*pattern == 0) {
    return str + len;
  } else {
    return 0;
  }
}

static inline const char* rev_dot(const char* str, const char* dot) {
  dot--;
  while(dot > str) {
    if(*dot == '.') {
      return dot + 1;
    }
    dot--;
  }
  return 0;
}

static const char* get_last_two_parts(const char* url) {
  if(url == 0) {
    return url;
  }
  int urllen = strlen(url);
  int dotcount = 0;
  for(int i = urllen - 1; i >= 0; --i) {
    if(*(url + i) == '.') {
      dotcount++;
    }
    if(dotcount == 2) {
      return (url + i + 1);
    }
  }
  return 0;
}

static const char* get_domain_from_site(const DomainDict* dict, const char* site) {
  const char* result = NULL;
  if(site == 0)
    return 0;
  int sitelen = strlen(site);
  if(sitelen == 0) {
    return 0;
  }
  const DomainDict::DomainList& t_top_domain = dict->get_top_domain();
  for(size_t i = 0; i < t_top_domain.size(); i++) {
    const char* match;
    match = rev_match(site, sitelen, t_top_domain[i].c_str());
    if(match != 0) {
      // �Զ���ͨ��������β���������ڶ��ڿ�ʼ�Ĳ�����Ϊ����
      result = rev_dot(site, match);
      // ����������ڣ�����������������Ϊ������
      if(result == 0) {
        return site;
      } else {
        return result;
      }
    }
  }
  const DomainDict::DomainListMap& t_domain_map = dict->get_country_domain_map();
  for(DomainDict::DomainListMap::const_iterator it = t_domain_map.begin();
      it != t_domain_map.end(); ++it) {
    // ������ļ�������������β������cn,tw,hk,uk,jp,il,nz,kr
    const char* match;
    match = rev_match(site, sitelen, it->first.c_str());
    if(match != 0) {
      const DomainDict::DomainList& t_second_domain = it->second;
      for(size_t i = 0; i < t_second_domain.size(); i++) {
        const char* match2;
        match2 = rev_match(site, sitelen, t_second_domain[i].c_str());
        if(match2 != 0) {
          // λ���ڶ��������б��У����Ե��������ڿ�ʼ�Ĳ�����Ϊ����
          result = rev_dot(site, match2);
          if(result == 0) {
            // ��������򷵻�����site
            return site;
          } else {
            return result;
          }
        }
      }
      goto next;
      // (zhangyan04):2012-1-4:�������ƥ�䲻�ϵĻ�����ô��������ͨ�ö�������.
//       // ���ڶ��������б��У�����������Ϊ����
//       result = rev_dot(site, match);
//       // ����������ڣ�����������������Ϊ������
//       if(result == 0) {
//         return site;
//       } else {
//         return result;
//       }
    }
  }
next:
  const DomainDict::DomainList& t_top_country = dict->get_top_country();
  const DomainDict::DomainList& t_general_2nd_domain = dict->get_general_2nd_domain();
  for(size_t i = 0; i < t_top_country.size(); i++) {
    // �Թ���������β
    const char* match;
    match = rev_match(site, sitelen, t_top_country[i].c_str());
    if(match != 0) {
      for(size_t j = 0; j < t_general_2nd_domain.size(); j++) {
        const char* match2;
        match2 = rev_match(site, match - site, t_general_2nd_domain[j].c_str());
        if(match2 != 0) {
          // �����ڶ���λ��general_2nd_domain�У�����������Ϊ����
          result = rev_dot(site, match2);
          if(result == 0) {
            // ��������򷵻�����site
            return site;
          } else {
            return result;
          }
        }
      }
      // ���ڶ��������б��У�����������Ϊ����
      result = rev_dot(site, match);
      // ����������ڣ�����������������Ϊ������
      if(result == 0) {
        return site;
      } else {
        return result;
      }
    }
  }
  // �Ȳ���ͨ��������β��Ҳ���Թ���������β��ȡ��������Ϊ���������������ڣ�������������Ϊ����
  return get_last_two_parts(site);
}

static inline int get_trunk_from_domain(const char* domain,
                                        char* trunk,
                                        int trunk_size) {
  int size = 0;
  while(domain[size] && domain[size] != '.') {
    size++;
  }
  if(size >= trunk_size) {
    return -1;
  }
  if(domain[size]) {
    memcpy(trunk, domain, size);
    trunk[size] = '\0';
  } else {
    trunk[0] = 0;
  }
  return 0;
}

// ���ж�16����
static bool is_legal_char(char ch) {
  // ��Ϊ������16���Ƶ�����
  if(isdigit(ch) || ch == '.' || ch == 'x' || ch == 'X' ||
      (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
    return true;
  }
  return false;
}


// �����ж��Ƿ�ΪIP
// ����cc.cc.cc.cc�����û�а취�ж�
// ������Բ���IP����ʹ�������������Ϊ��IP
// ����֮�����ǻ�Ҫ����һ���ж�.
static int guess_is_dotip(const char* site) {
  int dot_count = 0;
  while(*site) {
    if(!is_legal_char(*site)) {
      return 0;
    }
    if(*site == '.') {
      dot_count++;
    }
    site++;
  }
  if(dot_count == 3) {
    return 1; // ��IP,����ʵ��Ҳ���ܲ�ΪIP.
  }
  return 0; // ����IP
}

// ֻ���ж�10����
static bool is_legal_char2(char ch) {
  if(isdigit(ch) || ch == '.') {
    return true;
  }
  return false;
}

static int guess_is_dotip2(const char* site) {
  int dot_count = 0;
  while(*site) {
    if(!is_legal_char2(*site)) {
      return 0;
    }
    if(*site == '.') {
      dot_count++;
    }
    site++;
  }
  if(dot_count == 3) {
    return 1; // ��IP,����ʵ��Ҳ���ܲ�ΪIP.
  }
  return 0; // ����IP
}

static inline int get_value(const char* p, const char endc) {
  int val = 0;
  if(p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    p += 2;
    while(*p != endc) {
      char ch = *p;
      if(ch >= '0' && ch <= '9') {
        ch -= '0';
      } else if(ch >= 'a' && ch <= 'f') {
        ch -= 'a';
        ch += 10;
      } else if(ch >= 'A' && ch <= 'F') {
        ch -= 'A';
        ch += 10;
      } else {
        return -1;
      }
      val = val * 16 + ch;
      p++;
      //������ֲ���һ���Ϸ�����
      if(val >= 256) {
        return ILLEGAL_IP;
      }
    }
  } else if(p[0] == '0') { // oct.
    p += 1;
    while(*p != endc) {
      char ch = *p;
      if(ch >= '0' && ch <= '7') {
        ch -= '0';
      } else {
        return -1;
      }
      val = val * 8 + ch;
      p++;
      //������ֲ���һ���Ϸ�����
      if(val >= 256) {
        return ILLEGAL_IP;
      }
    }
  } else { // dec.
    while(*p != endc) {
      char ch = *p;
      if(ch >= '0' && ch <= '9') {
        ch -= '0';
      } else {
        return -1;
      }
      val = val * 10 + ch;
      p++;
      //������ֲ���һ���Ϸ�����
      if(val >= 256) {
        return ILLEGAL_IP;
      }
    }
  }
  return val;
}

// �ж����site�Ƿ����Ϊip�������ip�Ļ�������Ҫ����һ�ι�һ��
int ul_normalize_site_ip(const char* site,
                         char* result,
                         int result_size) {
  char tmp[256]; // I think it's enough.
  const char* saved_site = site;
  if(guess_is_dotip(site)) {
    int offset = 0;
    while(1) {
#if __GNUC__ == 4 && __GNUC_MINOR__ >= 6
      char* p = strchr((char *)site, '.');
#else
      char* p = strchr(site, '.');
#endif
      int val = 0;
      if(!p) { // last part.
        val = get_value(site, '\0');
        if(val == -1) {
          goto not_ip;
        } else if(val == ILLEGAL_IP) {
          return ILLEGAL_IP;
        }
        offset += snprintf(tmp + offset, sizeof(tmp) - offset, "%d", val);
        break;
      } else {
        val = get_value(site, '.');
        if(val == -1) {
          goto not_ip;
        } else if(val == ILLEGAL_IP) {
          return ILLEGAL_IP;
        }
        offset += snprintf(tmp + offset, sizeof(tmp) - offset, "%d.", val);
        site = p + 1;
      }
    }
    saved_site = tmp;
  }
not_ip:
  // same as domain
  int ret = snprintf(result, result_size, "%s", saved_site);
  if(ret < 0 || ret >= result_size) {
    return SITE_BUFFER_SMALL;
  }
  return SUCCESS;
}

static int ul_fetch_trunk_and_domain(const DomainDict* dict,
                                     const char* site,
                                     const char** domain,
                                     char* trunk,
                                     int trunk_size) {
  // �����IP
  if(guess_is_dotip2(site)) {
    *domain = site;
    // ����ṩtrunk�Ļ�
    if(trunk) {
      int ret = snprintf(trunk, trunk_size, "%s", site);
      if(ret < 0 || ret >= trunk_size) {
        return TRUNK_BUFFER_SMALL;
      }
    }
    return SUCCESS;
  }

  // �������IP
  const char* tmp_domain = get_domain_from_site(dict, site);
  // ������ǰ���'.'
  while(tmp_domain && *tmp_domain=='.'){
    tmp_domain++;
  }
  if(tmp_domain) {    
    *domain = tmp_domain;
    // ����ṩtrunk�Ļ�
    if(trunk) {
      if(get_trunk_from_domain(tmp_domain, trunk, trunk_size) != 0) {
        return TRUNK_BUFFER_SMALL;
      }
    }
    // OK.
    return SUCCESS;
  } else {
    // ��ȡdomain��������
    return ILLEGAL_DOMAIN;
  }
}

const char* ul_fetch_maindomain_ex(const char* site, char* trunk, int trunk_size,
                                   bool recoveryMode) {
  const char* domain = 0;
  static DomainDict dict;
  static bool init = false;
  if(!init) {
    dict.load_default();
    init = true;
  }
  if(ul_fetch_trunk_and_domain(&dict, site, &domain, trunk, trunk_size) != SUCCESS) {
    if(recoveryMode) {
      strncpy(trunk, site, trunk_size - 1);
      trunk[trunk_size-1] = 0;
      return site;
    }
    return NULL;
  }
  return domain;
}

const char* ul_fetch_maindomain_ex2(const char* site, char* trunk, int trunk_size,
                                    const DomainDict* dict, bool recoveryMode) {
  if(dict == NULL) {
    return ul_fetch_maindomain_ex(site, trunk, trunk_size);
  }
  const char* domain = 0;
  if(ul_fetch_trunk_and_domain(dict, site, &domain, trunk, trunk_size) != SUCCESS) {
    if(recoveryMode) {
      strncpy(trunk, site, trunk_size - 1);
      trunk[trunk_size-1] = 0;
      return site;
    }
    return NULL;
  }
  return domain;
}


#if 0

int main() {
  printf("# top domain\n");
  for(int i = 0; top_domain[i]; i++) {
    std::string s(top_domain[i]);
    reverse(s.begin(), s.end());
    printf("td %s\n", s.c_str());
  }
  printf("\n");

  printf("# top country\n");
  for(int i = 0; top_country[i]; i++) {
    std::string s(top_country[i]);
    reverse(s.begin(), s.end());
    printf("tc %s\n", s.c_str());
  }
  printf("\n");

  printf("# general 2nd domain\n");
  for(int i = 0; general_2nd_domain[i]; i++) {
    std::string s(general_2nd_domain[i]);
    reverse(s.begin(), s.end());
    printf("2nd %s\n", s.c_str());
  }
  printf("\n");

  printf("# country domain\n");
  for(int i = 0; domain_map[i].top_domain; i++) {
    std::string s1(domain_map[i].top_domain);
    reverse(s1.begin(), s1.end());
    const char** p = domain_map[i].second_domain;
    for(int j = 0; p[j]; j++) {
      std::string s2(p[j]);
      s2 = s2.substr(s1.length());
      reverse(s2.begin(), s2.end());
      printf("cd %s %s\n", s1.c_str(), s2.c_str());
    }
  }
  printf("\n");
  return 0;
}

#endif // #ifdef 0

