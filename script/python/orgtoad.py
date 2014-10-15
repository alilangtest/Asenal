#!/bin/env python2.6
#encoding=UTF-8

import sys, ldap, os
import urllib, urllib2, re, cookielib
import simplejson as json
import ldap
import ldap.modlist
import logging
import time
from time import strftime, localtime
from ad import Client, Creds, Locator, activate
from ad import AD_USERCTRL_NORMAL_ACCOUNT, AD_USERCTRL_ACCOUNT_DISABLED
from ad import Error as ADError
from ad import LDAPError

# Author: ChenZongzhi <baotiao@gmail.com>

# 全局变量
LDAP_HOST = '127.0.0.1'
#LDAP_PATH = "ldap://192.168.0.2:389/"
#LDAP_BASE_DN = 'sankuai.info'
#MGR_CRED = 'mistoadsync'
#MGR_PASSWD = 'mt#003003'
#LDAP_BASE_DN = 'meituan.info'
#MGR_CRED = 'Administrator'
#MGR_PASSWD = '87654312'
STOG_FILTER = 'description=4'
LOG_FILENAME = '/root/chenzongzhi/Adlog'
ENV = 0  #环境变量,判断是线上还是线下环境


if (ENV == 0):
    GROUP_URL = 'http://hr.sankuai.com/mtorg/org/api/ldap/getallorg'
    USER_URL = 'http://hr.sankuai.com/mtorg/org/api/ldap/getallemp'
    U_URL = 'http://hr.sankuai.com/mtorg/api/rtx/ldap/getallemp'
    PASSWD_URL = 'http://www.meituan.com/acl/sso/passwd/'
    DATA_URL = 'http://zzmt.dev.sankuai.com/acl/sso/getdata/'
    LDAP_BASE_DN = 'sankuai.info'
    MGR_CRED = 'mistoadsync'
    LDAP_PATH = "ldap://192.168.0.2:389/"
    MGR_PASSWD = 'mt#003003'

else:
    GROUP_URL = 'http://hr.sankuai.com/mtorg/org/api/ldap/getallorg'
    USER_URL = 'http://hr.sankuai.com/mtorg/org/api/ldap/getallemp'
    U_URL = 'http://hr.sankuai.com/mtorg/api/rtx/ldap/getallemp'
    PASSWD_URL = 'http://www.meituan.com/acl/sso/passwd/'
    DATA_URL = 'http://zzmt.dev.sankuai.com/acl/sso/getdata/'
    LDAP_BASE_DN = 'meituan.info'
    MGR_CRED = 'administrator'
    MGR_PASSWD = '87654312'
    LDAP_PATH = "ldap://192.168.180.100:389/"

def get_ou(stog_ou):
    path = str.split(stog_ou, '-')
    path.reverse()
    string = ""
    for group in path:
        if group == "0":
            continue
        string += 'ou=%s,' % (group)
    return string


def add_group():
    groups = get_data(0)
    creds = Creds(LDAP_BASE_DN)
    creds.acquire(MGR_CRED, MGR_PASSWD)
    activate(creds)

    client = Client(LDAP_BASE_DN)
    for group in groups:
        attrs = []
        stog_id = str(group['id'])
        stog_ou = group['path'].encode('utf-8')
        path = get_ou(stog_ou)
        dn = '%s%s' % (path, client.dn_from_domain_name(LDAP_BASE_DN))
        ou = group['name'].encode('utf-8')
        attrs.append(('ou', [ou]))
        attrs.append(('objectClass', ['top', 'organizationalUnit']))
        attrs.append(('description', [stog_id]))
        #path1 = search_group(stog_filter = 'description=' + stog_id)
        #print dn
        try:
            client.add(dn, attrs)
            logging.debug("[Group]Time %s add a new group, id is %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), ou)
        except (ADError, LDAPError):
            logging.debug("[Error Group]Time %s add group error, id is %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), ou)

def search_group(stog_filter = None, attrib = None):
    if not stog_filter:
            stog_filter = STOG_FILTER
    #print 'stog_filter is ', stog_filter
    creds = Creds(LDAP_BASE_DN)
    creds.acquire(MGR_CRED, MGR_PASSWD)
    activate(creds)
    client = Client(LDAP_BASE_DN)
    try:
        users = client.search(stog_filter, scheme='gc')
    except ldap.LDAPError, e:
        print e
    ans = ''
    if users:
        for attrs in users:
            ans = attrs[0]
            #print type(attrs)
            #print attrs[1]
            #print type(attrs[1]['whenChanged'])
            #print "Time is   ", attrs[1]['whenChanged']
            #return attrs[0]
    else:
        return None
    return ans
    

def add_user():
    users = get_user()
    creds = Creds(LDAP_BASE_DN)
    creds.acquire(MGR_CRED, MGR_PASSWD)
    activate(creds)

    client = Client(LDAP_BASE_DN)
    #sys.exit(users["data"])
    #print type(users)
    #print users["data"]
    for user in users["data"]:
        attrs = []
        stog_uid = user['uid'].encode('utf-8')
        stog_ou = str(user['ou']).encode('utf-8')
        telephoneNumber = str(user['mobile'])
        lo = user['l'].encode('utf-8')
        uid = str(user['uid'])
        c = user['c'].encode('utf-8')
        displayname = user['displayname'].encode('utf-8')
        cn = user['cn'].encode('utf-8')
        st = user['l'].encode('utf-8')
        employeeNumber = str(user['employeeNumber'])
        employeeType = user['employeeType'].encode('utf-8')
        mail = str(user['mail'])
        title = user['title'].encode('utf-8')
        sn = user['sn'].encode('utf-8')
        status = user['status']
        entryDN = str(user['entryDN']).encode('utf-8')
        if user['sex'] == 2:
            sex = '1'
        else:
            sex = '0'
        #user['sex'] == 2 ? sex = '0' : sex = '1'

        givenName = user['givenName'].encode('utf-8')
        #passwd = str(get_passwd(user['entryDN']))

        path = search_group(stog_filter = 'description=' + stog_ou)
        #print "path is ", path
        if path:
            dn = 'CN=%s,%s' % (stog_uid, path)
            #print "dn is ", dn

            path1 = search_group(stog_filter = 'cn=' + stog_uid)
            #print "path1 ", path1
            if path1 is None:
                attrs.append(('cn', [stog_uid]))
                attrs.append(('sAMAccountName', [stog_uid]))
                princ = '%s@%s' % (stog_uid, LDAP_BASE_DN)
                attrs.append(('userPrincipalName', [princ]))
                attrs.append(('description', [displayname]))
                ctrl = AD_USERCTRL_NORMAL_ACCOUNT | AD_USERCTRL_ACCOUNT_DISABLED
                attrs.append(('userAccountControl', [str(ctrl)]))
                attrs.append(('objectClass', ['user']))
                attrs.append(('Gender', [sex]))
                attrs.append(('co', [c]))
                attrs.append(('displayname', [displayname]))
                attrs.append(('sn', [sn]))
                attrs.append(('st', [st]))
                attrs.append(('givenName', [givenName]))
                attrs.append(('sk-employeeNumber', [employeeNumber]))
                attrs.append(('sk-employeeType', [employeeType]))
                attrs.append(('mail', [mail]))
                attrs.append(('title', [title]))
                attrs.append(('telephoneNumber', [telephoneNumber]))
                attrs.append(('mobile', [telephoneNumber]))
                try:
                    client.add(dn, attrs)
                    logging.debug("[User]Time %s add a new user, id is %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), stog_uid)
                except (ADError, LDAPError):
                    print ADError,LDAPError
                    logging.debug("[Error User]Time %s add user error, id is %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), stog_uid)

            else:
                print 'status', status
                if status == 128:
                    try:
                        client.rename(path1, 'cn=' + stog_uid, 'OU=已离职用户,OU=虚拟组,OU=总部,OU=美团,DC=sankuai,DC=info')
                        ctrl = 514
                        dnc = 'cn=' + stog_uid + ',OU=已离职用户,OU=虚拟组,OU=总部,OU=美团,DC=sankuai,DC=info'
                        mods = []
                        mods.append(('replace', 'userAccountControl', [str(ctrl)]))
                        client.modify(dnc, mods)
                        logging.debug("[leave office success] Time %s,old dn is %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), path1)
                        continue
                    except:
                        logging.debug("[leave office failed] Time %s,old dn is %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), path1)
                if cmp(path1, dn) != 0:
                    try:
                        client.rename(path1, 'cn=' + stog_uid, path)
                        logging.debug("[modrdn success] Time %s,old dn is %s, new dn is %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), path1, dn)
                    except:
                        logging.debug("[modrdn failed] Time %s,old dn is %s, new dn is %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), path1, dn)
                #attrs.append(('replace', 'cn', [stog_uid]))
                #attrs.append(('replace', 'sAMAccountName', [stog_uid]))
                princ = '%s@%s' % (stog_uid, LDAP_BASE_DN)
                attrs.append(('replace', 'userPrincipalName', [princ]))
                #ctrl = AD_USERCTRL_NORMAL_ACCOUNT | AD_USERCTRL_ACCOUNT_DISABLED
                #ctrl = AD_USERCTRL_NORMAL_ACCOUNT
                #attrs.append(('replace', 'userAccountControl', [str(ctrl)]))
                #attrs.append(('replace', 'objectClass', ['user']))
                attrs.append(('replace', 'Gender', [sex]))
                attrs.append(('replace', 'co', ['中国']))
                attrs.append(('replace', 'displayname', [displayname]))
                attrs.append(('replace', 'sn', [sn]))
                attrs.append(('replace', 'st', [st]))
                attrs.append(('replace', 'givenName', [givenName]))
                attrs.append(('replace', 'sk-employeeNumber', [employeeNumber]))
                attrs.append(('replace', 'sk-employeeType', [employeeType]))
                attrs.append(('replace', 'mail', [mail]))
                attrs.append(('replace', 'title', [title]))
                attrs.append(('replace', 'telephoneNumber', [str(0)]))
                attrs.append(('replace', 'mobile', [telephoneNumber]))
                attrs.append(('replace', 'description', [displayname]))
                try:
                    client.modify(dn, attrs)
                    logging.debug("[Modify User]Time %s add a new user, id is %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), stog_uid)
                except (ADError, LDAPError):
                    print ADError, LDAPError
                    logging.debug("[Modify User Error]Time %s add a new user, id is %s, error is %s, %s", strftime("%Y-%m-%d %H:%M:%S started", time.localtime()), stog_uid, ADError, LDAPError)

def get_data(type):
    cj = cookielib.CookieJar()
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
    exheaders = [("User-Agent","Mozilla/4.0 (compatible; MSIE 7.1; Windows NT 5.1; SV1)"),]
    opener.addheaders = exheaders
    response = urllib2.urlopen(DATA_URL + str(type))
    jsongroups = response.read()
    try:
        groups = json.loads(jsongroups)
    except e:
        print e
    #print groups
    return groups

def get_user():
    cj = cookielib.CookieJar()
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
    exheaders = [("User-Agent","Mozilla/4.0 (compatible; MSIE 7.1; Windows NT 5.1; SV1)"),]
    opener.addheaders = exheaders
    response = urllib2.urlopen(U_URL)
    jsongroups = response.read()
    try:
        groups = json.loads(jsongroups)
    except e:
        print e
    return groups

def get_passwd(uid):
    cj = cookielib.CookieJar()
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
    exheaders = [("User-Agent","Mozilla/4.0 (compatible; MSIE 7.1; Windows NT 5.1; SV1)"),]
    opener.addheaders = exheaders
    response = urllib2.urlopen(PASSWD_URL + str(uid))
    #print PASSWD_URL + str(uid)

    jsongroups = response.read()
    try:
        groups = json.loads(jsongroups)
    except e:
        print e
    #print "uid is", uid, "passwd is",groups
    #print groups
    return groups

def main():
    logging.basicConfig(filename = LOG_FILENAME, level = logging.DEBUG, filemod = 'w')
    #get_user()
    #add_group()
    #add_group()
    #add_group()
    #add_group()
    #add_group()
    #add_group()
    add_user()
    #search_group()


if __name__ == '__main__':
    main()
