"""PFuzz_web URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/2.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.conf.urls import include,url
from . import views
from . import settings
from django.views.static import serve


urlpatterns = [
    url('admin/', admin.site.urls),
    url(r'^$',views.get_programs_list),
    url(r'^index/$',views.get_programs_list),

    url(r'^newindex/$',views.newIndex),
    url(r'^update_des/$',views.update_des),
    url(r'^update_linkdb_des/$',views.update_linkdb_des),
    url(r'^update_addr/$',views.update_addr),
    url(r'^get_link_db/$',views.get_link_db),
    url(r'^get_task_details/$',views.get_task_details),

    url(r'^data_details/$',views.get_data),
    url(r'^testDB/$',views.test),
    url(r'^download/$',views.load_binary),
    url(r'^static/(?P<path>.*)$',serve,{'document_root':settings.MEDIA_ROOT}),

    url(r'^task_profile/$', views.task_profile),

    url(r'^home/$', views.get_programs_list),
    url(r'^get_programs_list/$', views.get_programs_list),
    url(r'^get_task_list/$', views.get_task_list),
    url(r'^update_linkdb_des/$', views.update_linkdb_des),
    url(r'^delete_program/$', views.delete_program),
    url(r'^delete_task/$', views.delete_task),
    url(r'^get_chart_data/$', views.get_chart_data),

]
